#ifndef CLOVER_GAME_WORLD_GEN_WORKER_HPP
#define CLOVER_GAME_WORLD_GEN_WORKER_HPP

#include "build.hpp"
#include "workerlocation.hpp"

namespace clover {
namespace game { namespace world_gen {

class WorkerType;

class Worker {
public:
	Worker(const WorkerType& type, WorkerLocation loc, real64 radius);
	
	WorkerLocation getLocation() const { return location; }
	real64 getRadius() const { return radius; }
	
	void work() const;

private:
	const WorkerType* type;
	WorkerLocation location;
	real64 radius;
};

}} // game::world_gen
} // clover

#endif // CLOVER_GAME_WORLD_GEN_WORKER_HPP