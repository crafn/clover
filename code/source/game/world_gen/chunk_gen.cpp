#include "chunk_gen.hpp"
#include "../world_mgr.hpp"
#include "../worldentity_mgr.hpp"
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
		, time(-inf)
{ }

void ChunkGen::generate(real64 to_time, const util::SlicedTask::Yield& yield)
{
	PROFILE();
	if (!initialized){
		PROFILE();
		for (const auto& w_type : owner->getWorkerTypes()){
			w_type->callChunkInit(*this);
		}
		initialized= true;
		yield();
	}

	while (!workers.empty() && getMinWorkerTime() <= to_time) {
		PROFILE();
		// Do some work
		for (auto it= workers.begin(); it != workers.end();) {
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

	yield();

	time= to_time;
	if (	getMinWorkerTime() >= 0.0 &&
			chunk->getState() == game::WorldChunk::State::Generating){
		PROFILE();
		// Chunk is ready
		ensure(chunk);
		chunk->setState(game::WorldChunk::State::Active);
	}
}

bool ChunkGen::isPlayable() const
{
	ensure(chunk);
	return initialized && chunk->getState() == game::WorldChunk::State::Active;
}

void ChunkGen::createEntity(const util::Str8& type_name, WorldVec position)
{
	ensure(owner);
	ensure(chunk);
	owner->getWorldMgr().getWeMgr().createEntity(type_name, position, chunk);
}

ChunkVec ChunkGen::getPosition() const
{
	ensure(chunk);
	return chunk->getPosition();
}

Worker& ChunkGen::createWorker(const util::Str8& name, WorldVec position, real64 radius, real64 time)
{
	/// @todo ensure that worker is inside chunk
	workers.emplaceBack(name, WorkerLocation{position, time, this}, radius);
	return workers.back();
}

void ChunkGen::addWorker(Worker w)
{
	workers.emplaceBack(std::move(w));
	workers.back().location.chunkGen= this;
}
real64 ChunkGen::getMinWorkerTime() const
{
	if (workers.empty())
		return inf;

	auto it= std::min_element(	workers.begin(), workers.end(),
								[] (const Worker& w1, const Worker& w2){
									return w1.getLocation().getTime() < w2.getLocation().getTime();
								});
	ensure(it != workers.end());
	return it->getLocation().getTime();
}

}} // game::world_gen
} // clover
