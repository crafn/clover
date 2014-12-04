#ifndef CLOVER_GAME_WORLD_GEN_WORLD_GEN_HPP
#define CLOVER_GAME_WORLD_GEN_WORLD_GEN_HPP

#include "build.hpp"
#include "chunk_gen.hpp"
#include "game/worldentity.hpp"
#include "script/script_mgr.hpp"
#include "util/linkedlist.hpp"
#include "util/set.hpp"
#include "util/slicedtask.hpp"

namespace clover {
namespace game {

class WorldMgr;
class WorldChunk;

} // game
namespace game { namespace world_gen {

class WorkerType;

/// Supervisor of world generation
class WorldGen {
public:
	using ChunkSet= util::Set<game::WorldChunk*>;
	static constexpr real64 eternity= util::SlicedTask::eternity;

	WorldGen(game::WorldMgr& w, util::DynArray<WorkerType*> worker_types);
	~WorldGen();

	game::WorldMgr& getWorldMgr() const { return *worldMgr; }

	/// Generates world approx max_real_dt seconds
	void generate(real64 max_real_dt);
	void addToGeneration(ChunkSet chunks, WorldVec priority_pos);

	const util::DynArray<WorkerType*>& getWorkerTypes() const { return workerTypes; }
	
	
	///
	/// Script interface
	///
	
	/// Only for global entities
	game::WeHandle createEntity(const util::Str8& name);

	game::WeHandle createEntity(const util::Str8& name, WorldVec position);
	game::WeHandle createEntity(const util::Str8& name, util::Vec3d position);
	
	/// Create entity with transform
	template <typename T>
	game::WeHandle createEntity(const util::Str8& name, T t);

	Worker& createWorker(	const util::Str8& name,
							WorldVec position,
							real64 radius,
							real64 start_time);

private:
	void generateChunks(const util::SlicedTask::Yield& yield);
	ChunkGen* getChunkGen(WorldVec worldpos);
	void transferWaitingWorkers(ChunkGen& gen);

	game::WorldMgr* worldMgr;
	util::DynArray<WorkerType*> workerTypes;
	bool initialized;
	util::LinkedList<ChunkGen> chunkGens;
	bool quit;
	
	util::SlicedTask generationTask;

	util::LinkedList<Worker> workersWaitingForChunk;
};

template <typename T>
game::WeHandle WorldGen::createEntity(const util::Str8& name, T t){
	game::WeHandle h= createEntity(name, t.translation.xy());
	h.ref().setAttribute("transform", t);
	return h;
}

}} // game::world_gen
namespace util {

template <>
struct TypeStringTraits<game::world_gen::WorldGen>{
	static util::Str8 type(){ return "world_gen::WorldGen"; }
};

} // util
} // clover

#endif // CLOVER_GAME_WORLD_GEN_WORLD_GEN_HPP
