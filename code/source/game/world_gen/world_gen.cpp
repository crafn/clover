#include "world_gen.hpp"
#include "../world_mgr.hpp"
#include "../worldchunk.hpp"
#include "../worldentity_mgr.hpp"
#include "util/containeralgorithms.hpp"
#include "util/profiling.hpp"
#include "workertype.hpp"

namespace clover {
namespace game { namespace world_gen {

WorldGen::WorldGen(game::WorldMgr& w, util::DynArray<WorkerType*> worker_types)
		: worldMgr(&w)
		, workerTypes(std::move(worker_types))
		, initialized(false)
		, quit(false)
		, generationTask(std::bind(&WorldGen::generateChunks, this, std::placeholders::_1))
{ }

WorldGen::~WorldGen()
{ }

void WorldGen::generate(real64 max_real_dt){
	PROFILE_("worldGen");
	if (!initialized){
		PROFILE();
		for (const auto& w_type : workerTypes){
			w_type->callGlobalInit(*this);
		}
		
		initialized= true;
	}

	// Playable chunks need to be always up-to-date so that
	// there's no discrepancies to be seen by the player
	for (auto& chunk : chunkGens) {
		if (!chunk.isPlayable())
			continue;
		PROFILE();
		chunk.generate(worldMgr->getTime(), util::SlicedTask::nullYield());
	}

	if (!generationTask.isFinished() && max_real_dt > 0.0){
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
		
		chunk->setState(game::WorldChunk::State::Generating);
		chunkGens.emplaceBack(*this, *chunk);
		transferWaitingWorkers(chunkGens.back());
	}
	
	if (generationTask.isFinished())
		generationTask.relaunch();
}

void WorldGen::stopGeneration()
{
	quit= true;
	generationTask.runFor(eternity);
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

Worker& WorldGen::createWorker(const util::Str8& name, WorldVec position, real64 radius, real64 time){
	PROFILE_("worldGen");
	/// @todo ensure that chunk isn't active yet

	ChunkGen* ch= getChunkGen(position);
	if (ch){
		return ch->createWorker(name, position, radius, time);
	}
	else {
		workersWaitingForChunk.pushBack(
				Worker{name, WorkerLocation{position, time, nullptr}, radius});
		return workersWaitingForChunk.back();
	}
}

void WorldGen::generateChunks(const util::SlicedTask::Yield& yield){
	PROFILE();
	while (!chunkGens.empty() && !quit){
		for (auto it= chunkGens.begin(); it != chunkGens.end(); ++it) {
			if (it->isPlayable())
				continue; // Playable chunks are generated elswehere
			it->generate(worldMgr->getTime(), yield);
			if (quit)
				break;
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
	util::LinkedList<Worker> workers= util::extractIf(workersWaitingForChunk, [&gen] (const Worker& w) -> bool {
		return game::WorldGrid::worldToChunkVec(w.getLocation().getPosition()) == gen.getPosition();
	});
	
	for (auto& w : workers)
		gen.addWorker(std::move(w));
}

}} // game::world_gen
} // clover
