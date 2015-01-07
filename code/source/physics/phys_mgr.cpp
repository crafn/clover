#include "armaturesuit.hpp"
#include "draw.hpp"
#include "fluid_mgr.hpp"
#include "global/cfg_mgr.hpp"
#include "global/env.hpp"
#include "joint.hpp"
#include "object_rigid.hpp"
#include "object.hpp"
#include "phys_mgr.hpp"
#include "util/profiling.hpp"
#include "world.hpp"

/// @todo Remove
#include "game/world_mgr.hpp"
#include "global/env.hpp"

namespace clover {
namespace physics {

util::ChunkMemPool& getRigidObjectPool()
{ return global::g_env.physMgr->getRigidObjectPool(); }

struct SimulationParams {
	real64 timeStep;
	int32 maxSteps;
	int32 velocityIterations;
	int32 positionIterations;
	real64 fluidTimeStep;
	bool useEstimation;
	real64 estimationTimeOffset;
};

SimulationParams getSimulationParamsFromCfg(){
	SimulationParams s;
	
	s.timeStep= global::g_env.cfg->get<real64>("physics::timeStep", 1.0/30.0);
	s.maxSteps= global::g_env.cfg->get<int32>("physics::maxStepsInFrame", 4);
	s.velocityIterations= global::g_env.cfg->get<int32>("physics::velocityIterations", 30);
	s.positionIterations= global::g_env.cfg->get<int32>("physics::positionIterations", 30);
	s.fluidTimeStep= global::g_env.cfg->get<real64>("physics::fluidTimeStep", 1.0/30.0);
	
	s.useEstimation= global::g_env.cfg->get<bool>("physics::useEstimation", true);
	s.estimationTimeOffset= global::g_env.cfg->get<real64>("physics::estimationTimeOffset", -0.5);

	return (s);
}

PhysMgr::PhysMgr()
		: rigidObjectMem(
				sizeof(RigidObject)*
				global::g_env.cfg->get<SizeType>(
					"physics::maxRigidObjectCount"),
				"rigidObjectMem")
		, rigidObjectPool(sizeof(RigidObject))
		, accumWorldTime(0.0)
		, accumFluidTime(0.0)
		, fluidSteps(0)
		, fluidTimeOffset(0.0)
		, fluidMgr(nullptr)
		, world(nullptr)
{
	// staticRigidObject needs this
	if (!global::g_env.physMgr)
		global::g_env.physMgr= this;

	rigidObjectPool.setMemory(&rigidObjectMem);

	listenForEvent(global::Event::OnPhysObjectCreate);
	listenForEvent(global::Event::OnPhysObjectDestroy);
	listenForEvent(global::Event::OnPhysJointCreate);
	listenForEvent(global::Event::OnPhysJointDestroy);
	listenForEvent(global::Event::OnPhysMaterialChange);

	util::Vec2d g= global::g_env.cfg->get<util::Vec2d>("physics::gravity");

	if (global::g_env.cfg->get("physics::enableFluids", true))
		fluidMgr= new FluidMgr{g};

	GridDef grid_def;
	grid_def.cellsInUnit= global::g_env.cfg->get("physics::gridCellsInUnit", 1);
	grid_def.chunkWidth= global::g_env.cfg->get("physics::gridChunkWidth", 16);

	world= new World{grid_def, g, fluidMgr};
	world->init();
}

PhysMgr::~PhysMgr(){
	int32 not_deleted= 0;

	for (int32 i=0; i<(int)objectTable.size(); i++){
		if (objectTable[i]){
			not_deleted++;
			delete objectTable[i];
			objectTable[i]= nullptr;
		}
	}

	if (not_deleted)
		print(debug::Ch::Phys, debug::Vb::Moderate, "PhysMgr::~PhysMgr(): objectTable contained %i PhysObjects", not_deleted);

	delete world;
	rigidObjectPool.setMemory(nullptr);

	delete fluidMgr;

	if (global::g_env.physMgr == this)
		global::g_env.physMgr= nullptr;
}

void PhysMgr::preFrameUpdate(){
	PROFILE_("physics");
	if (fluidMgr)
		fluidMgr->preUpdate();
}

void PhysMgr::update(){
	PROFILE_("physics");
	updateFrameTime();
	simulate(getSimulationParamsFromCfg());
}

void PhysMgr::fluidUpdate(){
	PROFILE_("physics");
	if (!fluidMgr)
		return;

	SimulationParams s= getSimulationParamsFromCfg();

	fluidTimeOffset= 0.0;
	if (s.useEstimation){
		fluidTimeOffset=
			s.estimationTimeOffset*s.fluidTimeStep + accumFluidTime;
	}

	if (fluidSteps > 0){
		fluidMgr->update(s.fluidTimeStep);
	}
}

void PhysMgr::postFrameUpdate(){
	PROFILE_("physics");
	if (fluidMgr)
		fluidMgr->postUpdate();
}

void PhysMgr::onEvent(global::Event& e){

	switch (e.getType()){
		/*case global::Event::OnPhysObjectCreate: {
			Object* o= e(global::Event::Object).getPtr<Object>();
			o->tableIndex= objectTable.insert(*o);
		}
		break;

		case global::Event::OnPhysObjectDestroy: {
			Object* o= e(global::Event::Object).getPtr<Object>();
			objectTable.remove(o->tableIndex);
			o->tableIndex= -1;
		}
		break;*/

		case global::Event::OnPhysJointCreate: {
			Joint* j= e(global::Event::Object).getPtr<Joint>();
			j->tableIndex= jointTable.insert(*j);

		}
		break;

		case global::Event::OnPhysJointDestroy: {
			Joint* j= e(global::Event::Object).getPtr<Joint>();
			jointTable.remove(j->tableIndex);
			j->tableIndex= -1;

		}
		break;

		case global::Event::OnPhysMaterialChange: {
			Material* mat= e(global::Event::Material).getPtr<Material>();
			for (auto& m : FixtureDef::getFixtureDefs()){
				if (m->getMaterial() == mat){
					m->onEvent(e);
				}
			}
		}
		
		default: break;
	}

}


int32 PhysMgr::calcObjectCount(){
	int32 count=0;
	for (int32 i=0; i<(int)objectTable.size(); i++)
		if (objectTable[i]) count++;

	return count;
}

World& PhysMgr::getWorld()
{ return *NONULL(world); }

void PhysMgr::updateFrameTime(){
	if (frameTimer.isRunning()){
		frameTimer.pause();

		real64 dt= frameTimer.getTime()*global::g_env.worldMgr->getTimeScale();
		accumWorldTime += dt;
		accumFluidTime += dt;

		frameTimer.reset();
	}
	frameTimer.run();
}

void PhysMgr::simulate(const SimulationParams& s){
	step(s);
	updateEstimations(s);
}

void PhysMgr::step(const SimulationParams& s){
	int32 steps= 0;
	while (accumWorldTime > s.timeStep && steps < s.maxSteps){
		accumWorldTime -= s.timeStep;
		++steps;
	}

	fluidSteps= 0;
	while (accumFluidTime > s.fluidTimeStep){
		accumFluidTime -= s.fluidTimeStep;
		++fluidSteps;
	}

	if (steps > 0){
		ArmatureSuit::preStepUpdates(s.timeStep*steps);
		
		for (int32 i= 0; i < steps; ++i){
			Joint::simulateFrictions(s.timeStep);
			world->step(s.timeStep, s.velocityIterations, s.positionIterations);
		}
		world->getB2World().ClearForces();
	}
}

void PhysMgr::updateEstimations(const SimulationParams& s){
	// Time relative to last physics step time scaled by 1/s.timeStep
	real64 relative_time= 0.0;
	
	if (s.useEstimation)
		relative_time= s.estimationTimeOffset + accumWorldTime/s.timeStep;
	
	if (relative_time > 1.0){
		print(debug::Ch::Phys, debug::Vb::Moderate, "Physics simulation falling behind (decrease quality, or increase maxStepsInFrame)");
		relative_time= 0.0;
		accumWorldTime= 0.0;
	}
	else if (relative_time < -1.0){
		print(debug::Ch::Phys, debug::Vb::Moderate, "PhysMgr::updateEstimations(..): invalid relative_time: %f", relative_time);
		relative_time= 0.0;
	}

	RigidObject::updateEstimationsForAll(relative_time);
}

} // physics
} // clover
