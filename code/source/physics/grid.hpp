#ifndef CLOVER_PHYSICS_GRID_HPP
#define CLOVER_PHYSICS_GRID_HPP

#include "build.hpp"
#include "util/arrayview.hpp"
#include "util/containeralgorithms.hpp"
#include "util/dyn_array.hpp"
#include "util/map.hpp"
#include "util/string.hpp"
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
class ENGINE_API Grid {
public:
	using ChunkVec= util::Vec2i;
	using CellVec= util::Vec2i;

	struct Cell {
		static constexpr SizeType maxObjects= 16;

		real64 staticPortion= 0.0;
		real64 lastStaticEdit= -std::numeric_limits<real64>::infinity(); // Game time
		real64 dynamicPortion= 0.0;
		real64 lastDynamicEdit= -std::numeric_limits<real64>::infinity();
		util::Vec2f staticNormal;
		bool staticEdge= false;
		bool worldEdge= false;
		Object* objects[maxObjects]= {};
	};

	Grid(GridDef def);

	void add(const Fixture& fix, util::RtTransform2d t);
	void remove(const Fixture& fix, util::RtTransform2d t);

	void clear();

	void touchCells(ChunkVec pos, real64 time);

	void addChunk(ChunkVec pos);
	void removeChunk(ChunkVec pos);
	bool hasChunk(ChunkVec pos) const;

	/// true if `shp` is in a region completely isolated by static objs
	bool isolated(const Shape& shp, util::RtTransform2d t);

	/// Chunk width in standard units
	uint32 getChunkWidth() const { return def.chunkWidth; }
	uint32 getCellsInUnit() const { return def.cellsInUnit; }

	const util::DynArray<Cell>& getChunkCells(ChunkVec v) const
	{ return chunks.at(v).cells; }
	Cell& getCell(util::Vec2d world_pos);

	util::DynArray<ChunkVec> getChunkPositions() const;
	
private:
	struct Chunk {
		util::DynArray<Cell> cells;
		/// False if chunk is not yet registered by `addChunk`
		bool valid= false;
		std::array<bool, 4> worldEdge= {};
	};
	using FilterCb= std::function<void (Grid&)>;
	enum class Action { add, remove };

	void updateWorldEdges(ChunkVec pos);
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
namespace util {

template <>
struct TypeStringTraits<physics::Grid> {
	static util::Str8 type() { return "physics::Grid"; }
};

template <>
struct TypeStringTraits<physics::Grid::Cell> {
	static util::Str8 type() { return "physics::GridCell"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_GRID_HPP
