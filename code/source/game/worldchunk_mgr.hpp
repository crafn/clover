#ifndef CLOVER_GAME_WORLDCHUNK_MGR_HPP
#define CLOVER_GAME_WORLDCHUNK_MGR_HPP

#include "build.hpp"
#include "util/set.hpp"
#include "worldchunk.hpp"

namespace clover {
namespace game {

/// Contains and controls world chunks
class WorldChunkMgr {
public:
	using ChunkSet= util::Set<game::WorldChunk*>;
	using ClusterSet= util::Set<ChunkSet>;
	
	ChunkSet getChunksInRadius(util::Vec2d p, real64 r);
	ChunkSet getChunksOutRadius(util::Vec2d p, real64 r);

	ChunkSet inhabitChunksInRadius(util::Vec2d p, real64 r);
	util::Set<RegionVec> getInhabitedRegionPositions() const;
	game::WorldChunk* getChunk(game::GridPoint p);
	game::WorldChunk& createChunk(game::GridPoint p);

	ChunkSet getChunks();
	
	/// @return Clusters that are fully inside the set of 'chunks'
	ClusterSet getCommonClusters(ChunkSet chunks) const;
	ClusterSet getClusters() const;
	
	/// Updates dependencies caused by WeHandles
	void updateChunkDependencies();
	
	/// Removing will take a few frames, doesn't cause choppiness
	void startDestroyingChunk(game::WorldChunk& c);
	void startDestroyingChunks(const ChunkSet& c);

	void updateDestroying();

	/// Destroys chunk and entitites contained
	void destroyChunk(game::WorldChunk& c);
	void destroyChunks(const ChunkSet& c);

	int32 getChunkCount(){ return chunks.size(); }

	void removeAll();

protected:
	util::Map<util::Vec2i, game::WorldChunk> chunks;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDCHUNK_MGR_HPP