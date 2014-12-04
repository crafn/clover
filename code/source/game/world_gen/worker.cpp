#include "chunk_gen.hpp"
#include "resources/cache.hpp"
#include "util/time.hpp"
#include "worker.hpp"
#include "workertype.hpp"

namespace clover {
namespace game { namespace world_gen {

Worker::Worker(const util::Str8& type_, WorkerLocation loc_, real64 radius_)
		: type(&resources::gCache->getResource<WorkerType>(type_))
		, location(loc_)
		, radius(radius_)
		, creationTime(util::gGameClock->getTime())
{ }

void Worker::work() const
{
	ensure(type);
	type->callWork(location.getChunkGen().getWorldGen(), *this);
}

}} // game::world_gen
} // clover
