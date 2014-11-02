#ifndef CLOVER_GAME_WORLDGRID_HPP
#define CLOVER_GAME_WORLDGRID_HPP

#include "build.hpp"
#include "util/math.hpp"

namespace clover {
namespace game {

/// All chunk/region coordinate transforms
/// @todo Reconsider more dynamic solution
class WorldGrid {
public:
	static constexpr uint32 chunkWidthInBlocks= 8; // x blokkia
	static constexpr uint32 regionWidthInChunks= 16; // x chunkkia
	static constexpr uint32 regionWidthInBlocks= chunkWidthInBlocks * regionWidthInChunks;
	
	static BlockVec worldToBlockVec(const WorldVec& p);
	static ChunkVec worldToChunkVec(const WorldVec& p);
	static RegionVec worldToRegionVec(const WorldVec& p);
	
	static WorldVec blockCornerToWorldVec(const BlockVec& p); 
	static WorldVec chunkCornerToWorldVec(const ChunkVec& p); 
	static WorldVec regionCornerToWorldVec(const RegionVec& p);
	
	static WorldVec blockCenterToWorldVec(const BlockVec& p); 
	static WorldVec chunkCenterToWorldVec(const ChunkVec& p); 
	static WorldVec regionCenterToWorldVec(const RegionVec& p);
};

class GridPoint {
public:
	GridPoint()= default;

	static GridPoint World(const WorldVec& p);
	static GridPoint Block(const BlockVec& p);
	static GridPoint Chunk(const ChunkVec& p);
	static GridPoint Region(const RegionVec& p);

	void setWorldVec(const WorldVec& p);
	void setBlockVec(const BlockVec& p);
	void setChunkVec(const ChunkVec& p);
	void setRegionVec(const RegionVec& p);

	WorldVec getWorldVec() const;
	BlockVec getBlockVec() const;
	ChunkVec getChunkVec() const;
	RegionVec getRegionVec() const;

private:
	WorldVec worldPosition;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDGRID_HPP