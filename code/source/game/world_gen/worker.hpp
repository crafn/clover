#ifndef CLOVER_GAME_WORLD_GEN_WORKER_HPP
#define CLOVER_GAME_WORLD_GEN_WORKER_HPP

#include "build.hpp"
#include "util/traits.hpp"
#include "workerlocation.hpp"

namespace clover {
namespace game { namespace world_gen {

class ChunkGen;
class WorkerType;

class Worker {
public:
	Worker(const util::Str8& type, WorkerLocation loc, real64 radius);
	
	/// @todo Remove, just have getPosition etc. directly here
	const WorkerLocation& getLocation() const { return location; }
	real64 getRadius() const { return radius; }
	real64 getCreationTime() const { return creationTime; }

	void work() const;

private:
	friend class ChunkGen;
	const WorkerType* type;
	WorkerLocation location;
	real64 radius;
	real64 creationTime;
};

}} // game::world_gen
namespace util {

template <>
struct TypeStringTraits<game::world_gen::Worker> {
	static util::Str8 type() { return "world_gen::Worker"; }
};

} // util
} // clover

#endif // CLOVER_GAME_WORLD_GEN_WORKER_HPP
