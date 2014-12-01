#ifndef CLOVER_PHYSICS_GRID_HPP
#define CLOVER_PHYSICS_GRID_HPP

#include "build.hpp"
#include "util/arrayview.hpp"
#include "util/containeralgorithms.hpp"
#include "util/dyn_array.hpp"
#include "util/map.hpp"
#include "util/traits.hpp"
#include "util/transform.hpp"
#include "util/vector.hpp"

namespace clover {
namespace physics {

class Fixture;
class Object;
class Shape;

struct GridDef {
	/// 1.0 / cell width
	uint32 cellsInUnit;
	/// Chunk width in standard units
	uint32 chunkWidth;
};

/// Spatial grid mirroring properties of physical entities 
/// Makes spatial queries cheap
class Grid {
public:
	using ChunkVec= util::Vec2i;
	using CellVec= util::Vec2i;

	struct Cell {
		static constexpr SizeType maxObjects= 16;

		real64 staticPortion= 0.0;
		real64 lastStaticPortion= 0.0;
		real64 dynamicPortion= 0.0;
		util::Vec2f staticNormal;
		bool staticEdge= false;
		Object* objects[maxObjects]= {};
	};

	Grid(GridDef def);

	void add(const Fixture& fix, util::RtTransform2d t);
	void remove(const Fixture& fix, util::RtTransform2d t);

	void clear();

	void update();

	void addChunk(ChunkVec pos);
	void removeChunk(ChunkVec pos);

	/// true if `shp` is in a region completely isolated by static objs
	bool isolated(const Shape& shp, util::RtTransform2d t);

	/// Chunk width in standard units
	uint32 getChunkWidth() const { return def.chunkWidth; }
	uint32 getCellsInUnit() const { return def.cellsInUnit; }

	util::ArrayView<const Cell> getChunkCells(ChunkVec v) const
	{ return util::asArrayView(chunks.at(v).cells); }
	Cell& getCell(util::Vec2d world_pos);

	util::DynArray<ChunkVec> getChunkPositions() const
	{ return util::keys(chunks); }
	
private:
	struct Chunk {
		util::DynArray<Cell> cells;
		/// False if chunk is not yet registered by `addChunk`
		bool valid= false;
	};
	using FilterCb= std::function<void (Grid&)>;
	enum class Action { add, remove };

	void modify(Action a, const physics::Fixture& fix, util::RtTransform2d t);
	Cell& getCell(CellVec cell_pos);
	Cell& getCell(Chunk& ch, uint32 x, uint32 y);

	uint32 cellsInChunkWidth() const
	{ return def.chunkWidth*def.cellsInUnit; }

	uint32 cellsInChunkArea() const
	{ return cellsInChunkWidth()*cellsInChunkWidth(); }

	real64 imprecision() const;

	GridDef def;
	util::Map<ChunkVec, Chunk> chunks;
};

/// Doesn't examine `obj` in any way
void add(Grid::Cell& cell, Object& obj);
void remove(Grid::Cell& cell, Object& obj);

} // physics
} // clover

#endif // CLOVER_PHYSICS_GRID_HPP
