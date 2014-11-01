#include "world_gen.hpp"
#include "../world_mgr.hpp"
#include "util/containeralgorithms.hpp"
#include "util/profiling.hpp"
#include "workertype.hpp"

namespace clover {
namespace game { namespace world_gen {

WorldGen::WorldGen(game::WorldMgr& w, util::DynArray<WorkerType*> worker_types)
		: worldMgr(&w)
		, workerTypes(std::move(worker_types))
		, initialized(false)
		, generationTask(std::bind(&WorldGen::generateChunks, this, std::placeholders::_1)){
}

WorldGen::~WorldGen(){
	ensure_msg(generationTask.isFinished(), "Generation isn't finished");
}

void WorldGen::generate(real64 max_real_dt){
	PROFILE_("worldGen");
	if (!initialized){
		PROFILE();
		for (const auto& w_type : workerTypes){
			w_type->callGlobalInit(*this);
		}
		
		initialized= true;
	}
	
	if (!generationTask.isFinished()){
		PROFILE();
		generationTask.runFor(max_real_dt);
	}
}

void WorldGen::addToGeneration(ChunkSet chunks, WorldVec priority_pos){
	PROFILE_("worldGen");
	auto priority_chunk= game::GridPoint::World(priority_pos).getChunkVec();
	for (const auto& chunk : game::WorldChunk::nearestSorted(priority_chunk, chunks)){
		ensure(chunk);
		ensure(chunk->getState() == game::WorldChunk::State::None);
		
		chunk->setState(game::WorldChunk::State::Creating);
		chunkGens.emplaceBack(*this, *chunk);
		transferWaitingWorkers(chunkGens.back());
	}
	
	if (generationTask.isFinished())
		generationTask.relaunch();
}

game::WeHandle WorldGen::createEntity(const util::Str8& name){
	ensure(worldMgr);
	return worldMgr->getWeMgr().createEntity(name);
}

game::WeHandle WorldGen::createEntity(const util::Str8& name, WorldVec position){
	ensure(worldMgr);
	return worldMgr->getWeMgr().createEntity(name, position);
}

game::WeHandle WorldGen::createEntity(const util::Str8& name, util::Vec3d position){
	game::WeHandle h= createEntity(name, position.xy());
	PROFILE();
	h.ref().setAttribute("transform", position);
	return h;
}

void WorldGen::createWorker(const util::Str8& name, WorldVec position, real64 radius, real64 time){
	PROFILE_("worldGen");
	/// @todo ensure that chunk isn't active yet
	
	ChunkGen* ch= getChunkGen(position);
	if (ch){
		ch->createWorker(name, position, radius, time);
	}
	else {
		workersWaitingForChunk.pushBack(WaitingWorker{name, position, radius, time});
	}
}

void WorldGen::generateChunks(const util::SlicedTask::Yield& yield){
	PROFILE();
	while (!chunkGens.empty()){
		chunkGens.front().generate(worldMgr->getTime(), yield);
		
		// Destroy ready chunk generators
		for (auto it= chunkGens.begin(); it != chunkGens.end();){
			if (it->isReady()){
				it= chunkGens.erase(it);
			}
			else {
				++it;
			}
		}
		
		yield();
	}
}

ChunkGen* WorldGen::getChunkGen(WorldVec worldpos){
	for (auto& gen : chunkGens){
		if (gen.getPosition() == game::WorldGrid::worldToChunkVec(worldpos)){
			return &gen;
		}
	}
	return nullptr;
}

void WorldGen::transferWaitingWorkers(ChunkGen& gen){
	util::LinkedList<WaitingWorker> workers= util::extractIf(workersWaitingForChunk, [&gen] (const WaitingWorker& w) -> bool {
		return game::WorldGrid::worldToChunkVec(w.position) == gen.getPosition();
	});
	
	for (auto& w : workers){
		gen.createWorker(w.name, w.position, w.radius, w.time);
	}
}

}} // game::world_gen
} // clover
