#include "chunk_util.hpp"
#include "collision/shape.hpp"
#include "fixture.hpp"
#include "grid.hpp"
#include "util/boundingbox.hpp"
#include "util/profiling.hpp"
#include "util/time.hpp"
// Debug
#include "game/physics.hpp"
#include "game/worldentity.hpp"

/// @todo Remove
#include "game/world_mgr.hpp"
#include "global/env.hpp"

namespace clover {
namespace physics {
namespace detail {

real64 timestamp()
{ return global::g_env.worldMgr->getTime(); }

SizeType cellIndex(Grid::CellVec cell, Grid::CellVec origin, Grid::CellVec size)
{
	Grid::CellVec p= cell - origin;
	SizeType i= p.x + p.y*size.x;
	ensure(i < static_cast<SizeType>(size.x*size.y));
	return i;
}

Grid::CellVec cellPosition(SizeType i, Grid::CellVec origin, Grid::CellVec size)
{
	using T= Grid::CellVec::Value;
	return Grid::CellVec{	static_cast<T>(i) % size.x,
							static_cast<T>(i)/size.x} + origin;
}

/// Cheap and inaccurate rasterization
util::DynArray<Grid::CellVec> rasterized(
		util::Vec2d a,
		util::Vec2d b,
		real64 cell_width)
{
	PROFILE();
	util::DynArray<Grid::CellVec> cells;

	real64 length= (b - a).length();
	real64 step_length= cell_width*0.1; // Arbitrary
	util::Vec2d step= (b - a).normalized()*step_length;
	SizeType steps= length/step_length;
	for (SizeType i= 0; i < steps; ++i) {
		Grid::CellVec cell_p= chunkVec(a, cell_width);
		if (cells.empty() || cells.back() != cell_p)
			cells.pushBack(cell_p);
		a += step;
	}

	return cells;	
}

struct PolyCell {
	Grid::CellVec position;
	real64 fill= 0.0;
	bool edge= false;
};

struct RowBounding {
	int32 min= std::numeric_limits<int32>::min();
	int32 max= std::numeric_limits<int32>::max();
};

util::DynArray<PolyCell> rasterized(	const util::Polygon& poly,
										uint32 cells_in_unit)
{
	PROFILE();
	SizeType v_count= poly.getVertexCount();
	if (v_count < 3)
		return {};

	// Bounding box
	util::BoundingBox<Grid::CellVec> bb;
	for (SizeType i= 0; i< v_count; ++i) {
		util::Vec2d p= poly.getVertex(i);
		Grid::CellVec cell_p= chunkVec(p, 1.0/cells_in_unit); 
		bb.append(cell_p);
	}
	Grid::CellVec grid_size= bb.getInclusiveSize();
	Grid::CellVec grid_origin= bb.getMin();

	// `cells` fill the whole bounding
	util::DynArray<PolyCell> cells;
	cells.resize(grid_size.x*grid_size.y);
	for (SizeType i= 0; i < cells.size(); ++i)
		cells[i].position= cellPosition(i, grid_origin, grid_size);

	// Used to skip over fully filled cells
	util::DynArray<RowBounding> rowBoundings;
	rowBoundings.resize(grid_size.y);

	// Used to calculate partially filled areas
	const uint32 subgrid_reso= 4;
	const uint32 subcells_in_unit= subgrid_reso*cells_in_unit;
	util::DynArray<RowBounding> subRowBoundings;
	subRowBoundings.resize(grid_size.y*subgrid_reso);

	// Pre-process polygon
	for (SizeType i= 0; i < v_count; ++i) {
		util::Vec2d cur_p= poly.getVertex(i);
		util::Vec2d next_p= poly.getVertex((i + 1) % v_count);
		real64 inv_slope= (next_p.x - cur_p.x)/(next_p.y - cur_p.y);

		// At which side are we
		bool left_side= cur_p.y > next_p.y;
		bool horizontal_segment= std::abs(cur_p.y - next_p.y) < util::epsilon;

		for (auto&& cell_p : rasterized(cur_p, next_p, 1.0/cells_in_unit)) {
			SizeType cell_i= cellIndex(cell_p, grid_origin, grid_size);
			ensure(cell_i < cells.size());

			{ // Adjust row boundings
				SizeType row_i= cell_p.y - grid_origin.y;
				ensure(row_i < rowBoundings.size());
				auto&& row= rowBoundings[row_i];
				if (horizontal_segment)
					row.min= row.max= cell_p.x;
				else if (left_side && row.min < cell_p.x)
					row.min= cell_p.x;
				else if (!left_side && row.max > cell_p.x)
					row.max= cell_p.x;
			}

			// Adjust sub-row boundings
			for (uint32 sub_i= 0; sub_i < subgrid_reso; ++sub_i) {
				real64 y= (cell_p.y + (sub_i + 0.5)/subgrid_reso)/cells_in_unit;
				real64 x= (y - cur_p.y)*inv_slope + cur_p.x;
				
				util::Vec2i subcell_p=
					chunkVec(	util::Vec2d{x, y},
								1.0/subcells_in_unit);

				SizeType row_i= subcell_p.y - grid_origin.y*subgrid_reso;
				ensure(row_i < subRowBoundings.size());
				auto&& row= subRowBoundings[row_i];
				if (horizontal_segment) {
					if ((y < cur_p.y && next_p.x > cur_p.x) ||
						(y > cur_p.y && next_p.x < cur_p.x)) {
						// Outside polygon
						row.min= subcell_p.x;
						row.max= row.min - 1;
					} else {
						// Inside polygon
						auto a_subcell= chunkVec(cur_p, 1.0/subcells_in_unit);
						auto b_subcell= chunkVec(next_p, 1.0/subcells_in_unit);
						if (a_subcell.x > b_subcell.x)
							std::swap(a_subcell, b_subcell);
						row.min= std::max(row.min, a_subcell.x);
						row.max= std::min(row.max, b_subcell.x);
					}
				}
				else if (left_side && row.min < subcell_p.x)
					row.min= subcell_p.x;
				else if (!left_side && row.max > subcell_p.x)
					row.max= subcell_p.x;
			}
		}
	}

	// Handle cells fully inside the polygon
	for (SizeType row_i= 0; row_i < rowBoundings.size(); ++row_i) {
		auto&& row= rowBoundings[row_i];
		if (	row.min > row.max || 
				row.min == std::numeric_limits<int32>::min() ||
				row.max == std::numeric_limits<int32>::max())
			continue; /// @todo Fix algorithm so this doesn't happen

		int32 y= grid_origin.y + row_i;
		for (int32 x= row.min + 1; x <= row.max - 1; ++x) {
			Grid::CellVec p{x, y};
			SizeType cell_i= cellIndex(p, grid_origin, grid_size);
			ensure(cell_i < cells.size());
			cells[cell_i].fill= 1.0;
		}
	}

	// Handle cells touching the edge of the polygon
	for (SizeType subrow_i= 0; subrow_i < subRowBoundings.size(); ++subrow_i) {
		auto&& subrow= subRowBoundings[subrow_i];
		if (	subrow.min > subrow.max || 
				subrow.min == std::numeric_limits<int32>::min() ||
				subrow.max == std::numeric_limits<int32>::max())
			continue;
	
		SizeType row_i= subrow_i/subgrid_reso;
		ensure(row_i < rowBoundings.size());
		auto&& row= rowBoundings[row_i];

		/// @todo Skip empty cells
		for (	int32 cell_x= grid_origin.x;
				cell_x < grid_origin.x + grid_size.x;
				++cell_x) {

			/// @todo Jump over full cells at once
			if (cell_x > row.min && cell_x < row.max)
				continue; // Cell is full and therefore already processed

			real64 area= 0.0;
			{
				int32 min_x= static_cast<int32>(cell_x*subgrid_reso);
				int32 max_x= static_cast<int32>((cell_x + 1)*subgrid_reso);
				int32 min= util::limited(subrow.min, min_x, max_x);
				// Exclude last subcell to prevent doubles when polygons touch
				int32 max= util::limited(subrow.max /* + 1*/, min_x, max_x);
				ensure(max >= min);
				area += max - min;
				area /= subgrid_reso*subgrid_reso;
			}

			Grid::CellVec cell_p{
				cell_x,
				grid_origin.y + static_cast<int32>(subrow_i/subgrid_reso),
			};
			SizeType cell_i= cellIndex(cell_p, grid_origin, grid_size);
			ensure(cell_i < cells.size());
			cells[cell_i].fill += area;
			cells[cell_i].edge= true;
		}
	}

	/// @todo Zero cell.fill if it's under some threshold
	
	return cells;
}

} // detail

Grid::Grid(GridDef def)
	: def(def)
{ }

void Grid::add(const physics::Fixture& fix, util::RtTransform2d t)
{
	PROFILE();
	modify(Action::add, fix, t);
}

void Grid::remove(const physics::Fixture& fix, util::RtTransform2d t)
{
	PROFILE();
	modify(Action::remove, fix, t);
}

void Grid::modify(Action a, const physics::Fixture& fix, util::RtTransform2d t)
{
	PROFILE();
	if (!fix.getShape())
		return;
	Object& obj= fix.getObject();
	collision::Shape shape;
	{ PROFILE();
		shape= *NONULL(fix.getShape());
		shape.transform(t);
	}
	bool is_static= fix.getObject().isStatic();
	util::BoundingBox<Grid::CellVec> bb;
	
	util::DynArray<util::Polygon> polys= shape.asConvexPolygons(imprecision());
	for (auto&& p : polys) {
		PROFILE();
		auto&& polycells= detail::rasterized(p, def.cellsInUnit);
		for (SizeType i= 0; i < polycells.size(); ++i) {
			auto&& cell= getCell(polycells[i].position);

			real64* cell_v_p= &cell.dynamicPortion;
			if (is_static) {
				cell_v_p= &cell.staticPortion;
			}
	
			real64& cell_v= *NONULL(cell_v_p);

			if (a == Action::add) {
				cell_v += polycells[i].fill;
			} else {
				cell_v -= polycells[i].fill;
				if (cell_v < 0.0)
					cell_v= 0.0;
			}

			if (polycells[i].fill != 0.0) {
				if (is_static)
					cell.lastStaticEdit= detail::timestamp();
				else
					cell.lastDynamicEdit= detail::timestamp();
			}

			if (a == Action::add) {
				physics::add(cell, obj);
			} else if (a == Action::remove) {
				physics::remove(cell, obj);
			}

			bb.append(polycells[i].position);
		}
	}

	// Post-process:
	//   - recalculate normals
	//   - falsify cells.staticEdge which really aren't edges
	for (int32 y= bb.getMin().y - 1; y < bb.getMax().y + 1; ++y) {
		PROFILE();
		for (int32 x= bb.getMin().x - 1; x < bb.getMax().x + 1; ++x) {
			util::Vec2i coord{x, y};
			const std::array<util::Vec2i, 4> dirs= {
				util::Vec2i{1, 0},
				util::Vec2i{0, 1},
				util::Vec2i{-1, 0},
				util::Vec2i{0, -1}
			};
			real64 static_fill= 0.0;
			util::Vec2f normal;
			for (auto& dir : dirs) {
				auto& side= getCell(coord + dir);
				real64 portion=
					side.staticPortion > 1.0 ? 1.0 : side.staticPortion;
				normal += -dir.casted<util::Vec2f>()*portion;
				static_fill += portion;
			}

			auto& cell= getCell(coord);
			cell.staticNormal= normal.normalized();

			bool prev_edge= cell.staticEdge;
			cell.staticEdge= static_fill < 0.8*4;
			if (prev_edge != cell.staticEdge)
				cell.lastStaticEdit= detail::timestamp();
		}
	}

}

void Grid::clear()
{
	chunks.clear();
}

void Grid::touchCells(ChunkVec pos, real64 time)
{
	auto& ch= chunks[pos];
	for (SizeType i= 0; i < ch.cells.size(); ++i) {
		auto& cell= ch.cells[i];
		cell.lastStaticEdit= time;
		cell.lastDynamicEdit= time;
	}
}

void Grid::addChunk(ChunkVec pos)
{
	chunks[pos].valid= true;
	chunks[pos].cells.resize(cellsInChunkArea());

	updateWorldEdges(pos);
}

void Grid::removeChunk(ChunkVec pos)
{
	auto it= chunks.find(pos);
	ensure(it != chunks.end());
	chunks.erase(it);

	updateWorldEdges(pos);
}

bool Grid::hasChunk(ChunkVec pos) const
{
	auto it= chunks.find(pos);
	return it != chunks.end() && it->second.valid;
}

bool Grid::isolated(const Shape& shp, util::RtTransform2d t)
{
	/// @todo Implementation
	ensure(0 && "@todo");
	return false;
}

auto Grid::getChunkPositions() const
-> util::DynArray<ChunkVec>
{
	util::DynArray<ChunkVec> ret;
	for (auto& p : chunks) {
		if (p.second.valid)
			ret.pushBack(p.first);
	}
	return ret;
}

void Grid::updateWorldEdges(ChunkVec pos)
{
	PROFILE();
	const std::array<util::Vec2i, 4> dirs= {
		util::Vec2i{1, 0},
		util::Vec2i{0, 1},
		util::Vec2i{-1, 0},
		util::Vec2i{0, -1}
	};
	const SizeType right= 0;
	const SizeType up= 1;
	const SizeType left= 2;
	const SizeType down= 3;

	util::DynArray<Chunk*> upd_chunks;

	Chunk* chunk= nullptr;
	if (chunks.find(pos) != chunks.end()) {
		chunk= &chunks[pos];
		upd_chunks.pushBack(chunk);
	}

	// Update chunk statuses
	for (SizeType i= 0; i < dirs.size(); ++i) {
		const auto& dir= dirs[i];

		Chunk* side= nullptr;
		if (chunks.find(pos + dir) != chunks.end())
			side= &chunks[pos + dir];
		if (side && !side->valid)
			side= nullptr;

		bool edge= (chunk == nullptr) != (side == nullptr);

		if (chunk)
			chunk->worldEdge[i]= edge;
		if (side) {
			side->worldEdge[(i + 2)%4]= edge;
			upd_chunks.pushBack(side);
		}
	}

	// Update cells at chunk edges
	SizeType width= cellsInChunkWidth();
	for (auto& ch_ptr : upd_chunks) {
		auto& ch= *NONULL(ch_ptr);

		// Corners
		getCell(ch, 0, 0).worldEdge=
			ch.worldEdge[left] || ch.worldEdge[down];
		getCell(ch, 0, width - 1).worldEdge= 
			ch.worldEdge[left] || ch.worldEdge[up];
		getCell(ch, width - 1, width - 1).worldEdge= 
			ch.worldEdge[right] || ch.worldEdge[up];
		getCell(ch, width - 1, 0).worldEdge= 
				ch.worldEdge[right] || ch.worldEdge[down];

		// Bottom
		for (SizeType x= 1; x < width - 1; ++x) {
			getCell(ch, x, 0).worldEdge= ch.worldEdge[down];
		}
		// Top
		for (SizeType x= 1; x < width - 1; ++x) {
			getCell(ch, x, width - 1).worldEdge= ch.worldEdge[up];
		}
		// Left
		for (SizeType y= 1; y < width - 1; ++y) {
			getCell(ch, 0, y).worldEdge= ch.worldEdge[left];
		}
		// Right
		for (SizeType y= 1; y < width - 1; ++y) {
			getCell(ch, width - 1, y).worldEdge= ch.worldEdge[right];
		}
	}
}


Grid::Cell& Grid::getCell(util::Vec2d world_pos)
{
	ChunkVec chunk_vec= chunkVec(world_pos, def.chunkWidth);
	auto&& chunk= chunks[chunk_vec];
	if (chunk.cells.size() < cellsInChunkArea())
		chunk.cells.resize(cellsInChunkArea());

	ChunkVec cell_vec=
		chunkVec(	world_pos - chunk_vec.casted<util::Vec2d>()*def.chunkWidth,
					1.0/def.cellsInUnit);

	SizeType cell_i= cell_vec.x + cell_vec.y*cellsInChunkWidth();
	ensure(cell_i < chunk.cells.size());
	return chunk.cells[cell_i];
}

Grid::Cell& Grid::getCell(CellVec cell_pos)
{
	/// @todo Remove unnecessary casting
	return getCell(	cell_pos.casted<util::Vec2d>()/def.cellsInUnit +
					util::Vec2d(0.5/def.cellsInUnit));
}

Grid::Cell& Grid::getCell(Chunk& ch, uint32 x, uint32 y)
{
	ensure((SizeType)(x + y*cellsInChunkWidth()) < ch.cells.size());
	return ch.cells[x + y*cellsInChunkWidth()];
}

real64 Grid::imprecision() const { return 1.0/def.cellsInUnit/2.0; }

void add(Grid::Cell& cell, Object& obj)
{
	bool added= false;
	for (SizeType i= 0; i < Grid::Cell::maxObjects; ++i) {
		// Really permissive due to `Grid::modify`
		if (	cell.objects[i] == nullptr ||
				cell.objects[i] == &obj) {
			cell.objects[i]= &obj;
			added= true;
			break;
		}
	}
	if (!added) {
		util::Str8 we_name= "-";
		auto pos= obj.getPosition();
		game::WorldEntity* we= game::getOwnerWe(obj);
		if (we)
			we_name= we->getTypeName();
		print(	debug::Ch::Phys, debug::Vb::Critical,
				"Too many objects in a physics::Grid::Cell! WeType: %s, pos: %f, %f",
				we_name.cStr(),
				pos.x,
				pos.y);
	}
}

void remove(Grid::Cell& cell, Object& obj)
{
	for (SizeType i= 0; i < Grid::Cell::maxObjects; ++i) {
		// Really permissive due to `Grid::modify`
		if (cell.objects[i] == &obj) {
			cell.objects[i]= nullptr;
			break;
		}
	}
}

} // physics
} // clover
