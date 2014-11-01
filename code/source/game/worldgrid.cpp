#include "worldgrid.hpp"

namespace clover {
namespace game {

constexpr uint32 WorldGrid::chunkWidthInBlocks;
constexpr uint32 WorldGrid::regionWidthInChunks;
constexpr uint32 WorldGrid::regionWidthInBlocks;

BlockVec WorldGrid::worldToBlockVec(const WorldVec& pos){
	return BlockVec { (int32)floor(pos.x), (int32)floor(pos.y) };
}

ChunkVec WorldGrid::worldToChunkVec(const WorldVec& pos){
	return ChunkVec { (int32)floor((real32)pos.x/chunkWidthInBlocks), (int32)floor((real32)pos.y/chunkWidthInBlocks)};
}

RegionVec WorldGrid::worldToRegionVec(const WorldVec& pos){
	return RegionVec { (int32)floor((real32)pos.x/regionWidthInBlocks), (int32)floor((real32)pos.y/regionWidthInBlocks)};
}

WorldVec WorldGrid::blockCornerToWorldVec(const BlockVec& p){
	return WorldVec { (real64)p.x, (real64)p.y };
}

WorldVec WorldGrid::chunkCornerToWorldVec(const ChunkVec& p){
	return WorldVec { (real64)p.x*chunkWidthInBlocks, (real64)p.y*chunkWidthInBlocks };
}

WorldVec WorldGrid::regionCornerToWorldVec(const RegionVec& p){
	return WorldVec { (real64)p.x*regionWidthInBlocks, (real64)p.y*regionWidthInBlocks };
}

WorldVec WorldGrid::blockCenterToWorldVec(const BlockVec& p){
	return WorldVec { (real64)p.x + 0.5, (real64)p.y + 0.5 };
}

WorldVec WorldGrid::chunkCenterToWorldVec(const ChunkVec& p){
	return WorldVec { (real64)(p.x + 0.5)*chunkWidthInBlocks, (real64)(p.y + 0.5)*chunkWidthInBlocks };
}

WorldVec WorldGrid::regionCenterToWorldVec(const RegionVec& p){
	return WorldVec { (real64)(p.x + 0.5)*regionWidthInBlocks, (real64)(p.y + 0.5)*regionWidthInBlocks };
}

GridPoint GridPoint::World(const WorldVec& p){
	GridPoint point;
	point.setWorldVec(p);
	return point;
}

GridPoint GridPoint::Block(const BlockVec& p){
	GridPoint point;
	point.setBlockVec(p);
	return point;
}

GridPoint GridPoint::Chunk(const ChunkVec& p){
	GridPoint point;
	point.setChunkVec(p);
	return point;
}

GridPoint GridPoint::Region(const RegionVec& p){
	GridPoint point;
	point.setRegionVec(p);
	return point;
}

void GridPoint::setWorldVec(const WorldVec& p){
	worldPosition= p;
}

void GridPoint::setBlockVec(const BlockVec& p){
	worldPosition= WorldGrid::blockCornerToWorldVec(p);
}

void GridPoint::setChunkVec(const ChunkVec& p){
	worldPosition= WorldGrid::chunkCornerToWorldVec(p);
}

void GridPoint::setRegionVec(const RegionVec& p){
	worldPosition= WorldGrid::regionCornerToWorldVec(p);
}

WorldVec GridPoint::getWorldVec() const {
	return worldPosition;
}

BlockVec GridPoint::getBlockVec() const {
	return WorldGrid::worldToBlockVec(worldPosition);
}

ChunkVec GridPoint::getChunkVec() const {
	return WorldGrid::worldToChunkVec(worldPosition);
}

RegionVec GridPoint::getRegionVec() const {
	return WorldGrid::worldToRegionVec(worldPosition);
}

} // game
} // clover