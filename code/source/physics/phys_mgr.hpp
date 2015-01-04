#ifndef CLOVER_PHYSICS_PHYSMGR_HPP
#define CLOVER_PHYSICS_PHYSMGR_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "util/math.hpp"
#include "util/mem_pool_chunk.hpp"
#include "util/time.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace physics {

class Draw;
class FluidMgr;
class Joint;
class Object;
class RigidObject;
class SimulationParams;
class World;

/// Manages Objects, Joints and Worlds
class PhysMgr : public global::EventReceiver {
public:
	PhysMgr();
	virtual ~PhysMgr();

	void preFrameUpdate();
	void update();
	void fluidUpdate();
	void postFrameUpdate();

	void onEvent(global::Event& e);

	int32 calcObjectCount();
	World& getWorld();

	FluidMgr* getFluidMgr() { return fluidMgr.get(); }

	/// Offset from current state to match world time
	real64 getFluidTimeOffset() const { return fluidTimeOffset; }

	util::ChunkMemPool& getRigidObjectPool() { return rigidObjectPool; }

private:
	void updateFrameTime();

	/// Fully updates physics state
	void simulate(const SimulationParams& s);

	void step(const SimulationParams& s);
	void updateEstimations(const SimulationParams& s);

	util::MemChunk rigidObjectMem;
	util::ChunkMemPool rigidObjectPool;

	util::PtrTable<Object> objectTable;
	util::PtrTable<Joint> jointTable;

	util::Timer frameTimer;

	real64 accumWorldTime;
	real64 accumFluidTime;
	int32 fluidSteps;
	real64 fluidTimeOffset;

	util::UniquePtr<FluidMgr> fluidMgr;
	util::UniquePtr<World> world;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_PHYSMGR_HPP
