#ifndef CLOVER_GAME_WORLDCHUNKTASK_HPP
#define CLOVER_GAME_WORLDCHUNKTASK_HPP

#include "build.hpp"
#include "worldgrid.hpp"

namespace clover {
namespace game {

/// Chunk operation sliced over multiple frames
class WorldChunkTask {
public:
	WorldChunkTask(ChunkVec chunk_pos): chunkPosition(chunk_pos){}
	virtual ~WorldChunkTask(){}

	enum class UpdateState {
		Finished,   /// Task has done everything it is supposed to
		Unfinished, /// More updates needed to finish
		Blocking    /// Not finished, but waiting for some resource which is not available yet
	};

	virtual UpdateState update()= 0;

	ChunkVec getChunkPosition() const { return chunkPosition; }

private:
	ChunkVec chunkPosition;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDCHUNKTASK_HPP