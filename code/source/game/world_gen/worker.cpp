#include "worker.hpp"
#include "chunk_gen.hpp"
#include "workertype.hpp"

namespace clover {
namespace game { namespace world_gen {

Worker::Worker(const WorkerType& type_, WorkerLocation loc_, real64 radius_)
		: type(&type_)
		, location(loc_)
		, radius(radius_){
}

void Worker::work() const {
	ensure(type);
	type->callWork(location.getChunkGen().getWorldGen(), location);
}

}} // game::world_gen
} // clover