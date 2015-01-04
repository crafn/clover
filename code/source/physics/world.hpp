#ifndef CLOVER_PHYSICS_WORLD_HPP
#define CLOVER_PHYSICS_WORLD_HPP

#include "build.hpp"
#include "contactlistener.hpp"
#include "grid.hpp"
#include "util/polygon.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace physics {

class FluidMgr;
class RigidObject;
class Material;

class World {
public:
	World(GridDef grid_def, util::Vec2d gravity, FluidMgr* fluid_mgr);
	virtual ~World();
	
	void init();
	void step(real64 dt, int32 vel_iterations, int32 pos_iterations);

	/// Speeds things up
	void applyRadialImpulseField(
			util::Vec2d center, real64 magnitude, real64 halfvaluedistance);

	/// Breaks things
	/// Parameters are somewhat consistent with applyRadialImpulse:
	///		If e.g. explosion happens, you should apply impulse and
	///		stress field with the same parameters
	void applyRadialStressField(
			util::Vec2d center, real64 magnitude, real64 halfvaluedistance);

	b2World& getB2World(){ return *box2dWorld; }
	RigidObject& getStaticRigidObject(){ return *staticRigidObject; }
	util::Vec2d getGravity() const { return gravity; }

	/// These chunks should match with the grid
	void addChunk(util::Vec2i chunk_pos, uint32 chunk_width);
	void removeChunk(util::Vec2i chunk_pos, uint32 chunk_width);

	Grid& getGrid(){ return grid; }

private:
	struct BreakingCollision {
		Object* collider= nullptr;
		// Breaking material
		const Material* material= nullptr;

		// From collider
		util::Vec2d normal;
		real64 impulse;
		util::Vec2d hitPoint;
		real64 obtusionCircleRadius;
		real64 colliderObtusion;
		util::DynArray<util::Polygon> colliderPolys;
		real64 maxPolyRadius;
	};

	static util::DynArray<BreakingCollision> getBreakingCollisions(
			const util::DynArray<PostSolveContact>& contacts);
	void processBreakingContacts();
	void breakByCollision(const BreakingCollision& col);
	util::Vec2i chunkToFluidChunkPos(util::Vec2i chunk_pos) const;

	b2World* box2dWorld= nullptr;
	RigidObject* staticRigidObject= nullptr;
	Grid grid;
	util::Vec2d gravity;

	InternalContactListener contactListener;

	FluidMgr* fluidMgr;
	// <Fluid chunk pos, count of chunks inside fluid chunk>
	util::Map<util::Vec2i, int32> fluidChunkCounters;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_WORLD_HPP
