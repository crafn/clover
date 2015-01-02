#include "phys_mgr.hpp"
#include "armaturesuit.hpp"
#include "draw.hpp"
#include "global/cfg_mgr.hpp"
#include "global/env.hpp"
#include "joint.hpp"
#include "object_rigid.hpp"
#include "object.hpp"
#include "util/profiling.hpp"
#include "world.hpp"

namespace clover {
namespace physics {

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
	
	s.timeStep= global::gCfgMgr->get<real64>("physics::timeStep", 1.0/30.0);
	s.maxSteps= global::gCfgMgr->get<int32>("physics::maxStepsInFrame", 4);
	s.velocityIterations= global::gCfgMgr->get<int32>("physics::velocityIterations", 30);
	s.positionIterations= global::gCfgMgr->get<int32>("physics::positionIterations", 30);
	s.fluidTimeStep= global::gCfgMgr->get<real64>("physics::fluidTimeStep", 1.0/30.0);
	
	s.useEstimation= global::gCfgMgr->get<bool>("physics::useEstimation", true);
	s.estimationTimeOffset= global::gCfgMgr->get<real64>("physics::estimationTimeOffset", -0.5);

	return (s);
}


PhysMgr::PhysMgr()
		: rigidObjectPoolMem(
				sizeof(RigidObject)*
				global::gCfgMgr->get<SizeType>(
					"physics::maxRigidObjectCount"),
				"rigidObjectMem")
		, accumWorldTime(0.0)
		, accumFluidTime(0.0)
		, fluidSteps(0)
		, fluidTimeOffset(0.0){
	
	// staticRigidObject needs this
	if (!global::g_env.physMgr)
		global::g_env.physMgr= this;

	RigidObject::setPoolMem(&rigidObjectPoolMem);

	listenForEvent(global::Event::OnPhysObjectCreate);
	listenForEvent(global::Event::OnPhysObjectDestroy);
	listenForEvent(global::Event::OnPhysJointCreate);
	listenForEvent(global::Event::OnPhysJointDestroy);
	listenForEvent(global::Event::OnPhysMaterialChange);

	util::Vec2d g= global::gCfgMgr->get<util::Vec2d>("physics::gravity");

	if (global::gCfgMgr->get("physics::enableFluids", true))
		fluidMgr.emplace(g);

	GridDef grid_def;
	grid_def.cellsInUnit= global::gCfgMgr->get("physics::gridCellsInUnit", 1);
	grid_def.chunkWidth= global::gCfgMgr->get("physics::gridChunkWidth", 16);
	gWorld= new World(grid_def, g, fluidMgr.get());
	gWorld->init();
}


PhysMgr::~PhysMgr(){
	delete gWorld; gWorld= nullptr;

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

	RigidObject::setPoolMem(nullptr);

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

World& PhysMgr::getWorld(){
	return *gWorld;
}

void PhysMgr::updateFrameTime(){
	if (frameTimer.isRunning()){
		frameTimer.pause();

		real64 dt= frameTimer.getTime()*util::gGameClock->getTimeScale();
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
			gWorld->step(s.timeStep, s.velocityIterations, s.positionIterations);
		}
		gWorld->getB2World().ClearForces();
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
