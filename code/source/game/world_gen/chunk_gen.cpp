#include "chunk_gen.hpp"
#include "../world_mgr.hpp"
#include "../worldchunk.hpp"
#include "util/profiling.hpp"
#include "world_gen.hpp"
#include "workertype.hpp"

namespace clover {
namespace game { namespace world_gen {

ChunkGen::ChunkGen(WorldGen& o, game::WorldChunk& c)
		: initialized(false)
		, owner(&o)
		, chunk(&c)
		, time(0.0){
}

void ChunkGen::generate(real64 to_time, const util::SlicedTask::Yield& yield){
	PROFILE();
	if (!initialized){
		PROFILE();
		for (const auto& w_type : owner->getWorkerTypes()){
			w_type->callChunkInit(*this);
		}
		initialized= true;
		yield();
	}

	while (!workers.empty() && getMinWorkerTime() <= to_time){
		PROFILE();
		// Do some work
		for (auto it= workers.begin(); it != workers.end();){
			if (it->getLocation().getTime() <= to_time){
				PROFILE();
				it->work();
				it= workers.erase(it);
				break;
			}
			else {
				++it;
			}
		}
		
		{ PROFILE();
			yield();
		}
	}

	time= to_time;
	
	yield();
	
	if (workers.empty()){
		PROFILE();
		// Chunk is ready
		ensure(chunk);
		chunk->setState(game::WorldChunk::State::Active);
	}
}

bool ChunkGen::isReady() const {
	ensure(chunk);
	return initialized && chunk->getState() == game::WorldChunk::State::Active;
}

void ChunkGen::createEntity(const util::Str8& type_name, WorldVec position){
	ensure(owner);
	ensure(chunk);
	owner->getWorldMgr().getWeMgr().createEntity(type_name, position, chunk);
}

ChunkVec ChunkGen::getPosition() const {
	ensure(chunk);
	return chunk->getPosition();
}

void ChunkGen::createWorker(const util::Str8& name, WorldVec position, real64 radius, real64 time){
	/// @todo ensure that worker is inside chunk
	workers.emplaceBack(resources::gCache->getResource<WorkerType>(name), WorkerLocation{position, time, *this}, radius);
}

real64 ChunkGen::getMinWorkerTime() const {
	auto it=std::min_element(	workers.begin(), workers.end(),
								[] (const Worker& w1, const Worker& w2){
									return w1.getLocation().getTime() < w2.getLocation().getTime();
								});
	ensure(it != workers.end());
	return it->getLocation().getTime();
}

}} // game::world_gen
} // clover