#ifndef CLOVER_GAME_WORLD_GEN_CHUNK_GEN_HPP
#define CLOVER_GAME_WORLD_GEN_CHUNK_GEN_HPP

#include "build.hpp"
#include "util/linkedlist.hpp"
#include "util/slicedtask.hpp"
#include "worker.hpp"

namespace clover {
namespace game {

class WorldChunk;

} // game
namespace game { namespace world_gen {

class WorldGen;

/// Controls world generation of one chunk
class ChunkGen {
public:
	static constexpr real64 inf= std::numeric_limits<real64>::infinity();

	ChunkGen(WorldGen& owner, game::WorldChunk& chunk);
	
	void generate(real64 to_time, const util::SlicedTask::Yield& yield);
	
	/// Chunk is generated (but there can still be workers)
	bool isPlayable() const;
	
	///
	/// Script interface
	///

	WorldGen& getWorldGen() const { ensure(owner); return *owner; }

	/// Creates entity in this chunk
	void createEntity(const util::Str8& type_name, WorldVec position);
	ChunkVec getPosition() const;
	
	Worker& createWorker(	const util::Str8& name,
							WorldVec position,
							real64 radius,
							real64 start_time);
	void addWorker(Worker w);
	
private:
	real64 getMinWorkerTime() const;

	bool initialized;
	WorldGen* owner;
	game::WorldChunk* chunk;
	
	real64 time; /// Time of generation. Zero is start of the game (= fully generated)
	util::LinkedList<Worker> workers;
};

}} // game::world_gen
} // clover

#endif // CLOVER_GAME_WORLD_GEN_CHUNK_GEN_HPP
