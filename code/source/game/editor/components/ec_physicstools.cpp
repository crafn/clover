#include "ec_physicstools.hpp"
#include "collision/query.hpp"
#include "debug/draw.hpp"
#include "global/env.hpp"
#include "gui/cursor.hpp"
#include "gui/gui_mgr.hpp"
#include "hardware/mouse.hpp"
#include "hardware/keyboard.hpp"
#include "physics/joints.hpp"
#include "ui/game/base_ui.hpp"

namespace clover {
namespace game { namespace editor {

PhysicsToolsEc::PhysicsToolsEc()
		: startDragL("host", "dev", "phys_startGrab",
				[this] (){ onGrabAction(true); })
		, stopDragL("host", "dev", "phys_stopGrab",
				[this] (){ onGrabAction(false); })
		, addJointL("host", "dev", "phys_addJoint",
				[this] (){ onAddJointAction(); })
		, prevJointTypeL("host", "dev", "phys_prevJointType",
				[this] (){ onChangeJointTypeAction(-1); })
		, nextJointTypeL("host", "dev", "phys_nextJointType",
				[this] (){ onChangeJointTypeAction(1); }){
}

void PhysicsToolsEc::update(){
	if (grabJoint){
		grabJoint.setTarget(cursorOnWorld());
	}

	/*for (util::UniquePtr<physics::Joint>& j : joints){
		physics::DistanceJoint* d= dynamic_cast<physics::DistanceJoint*>(j.get());
		if (d){
			b2Vec2 f= d->getB2Joint()->GetReactionForce(0.1);
			print(debug::Ch::General, debug::Vb::Trivial, "%f, %f", f.x, f.y);
		}
	}*/	
}

void PhysicsToolsEc::setDrawingActive(bool b){
	global::g_env->debugDraw->setEnabled(debug::Draw::DrawFlag::Physics, b);
}

bool PhysicsToolsEc::isDrawingActive() const {
	return global::g_env->debugDraw->isEnabled(debug::Draw::DrawFlag::Physics);
}

void PhysicsToolsEc::setDrawingFlag(physics::Draw::Flag f, bool b){
	global::g_env->debugDraw->getPhysicsDraw().setFlag(f, b);
}

bool PhysicsToolsEc::getDrawingFlag(physics::Draw::Flag f) const {
	return global::g_env->debugDraw->getPhysicsDraw().getFlag(f);
}

void PhysicsToolsEc::setDrawingAlpha(real32 a){
	global::g_env->debugDraw->getPhysicsDraw().setAlpha(a);
}

real32 PhysicsToolsEc::getDrawingAlpha() const {
	return global::g_env->debugDraw->getPhysicsDraw().getAlpha();
}

physics::Draw::Flag PhysicsToolsEc::getFlagEnum(int32 i) const {
	return global::g_env->debugDraw->getPhysicsDraw().getFlagEnum(i);
}

void PhysicsToolsEc::onGrabAction(bool start){
	gui::Cursor& guiCursor= global::g_env->guiMgr->getCursor();

	if (start){
		if(		guiCursor.getTouchedElement() ==
				&global::g_env->ui->getInGameUi()->getWorldElement()){

			util::DynArray<physics::Object*> objects;

			auto lambda= [&] (physics::Fixture& t){
				objects.pushBack(&t.getObject());
				return true;
			};

			collision::Query::fixture.point(cursorOnWorld(), lambda);

			if (!objects.empty()){
				grabJoint.setMaxForce(objects.front()->getMass()*5000);
				grabJoint.setFrequency(5.0);
				grabJoint.setDamping(1.3);
				grabJoint.attach(*objects.front(), cursorOnWorld());
				grabJoint.setTarget(cursorOnWorld());
			}
		}
	}
	else {
		// Stop
		grabJoint.detach();
	}
}

void PhysicsToolsEc::onAddJointAction(){
	util::DynArray<physics::Object*> objects;

	auto lambda= [&] (physics::Fixture& t){
		objects.pushBack(&t.getObject());
		return true;
	};

	collision::Query::fixture.point(cursorOnWorld(), lambda);

	if (currentJointType == JointType::Detach){
		if (!objects.empty()){
			physics::Object& obj= *NONULL(objects.front());
			
			// Remove editor joints belonging to clicked object
			util::DynArray<physics::Joint*> obj_joints= obj.getJoints();
			for (auto joint : obj_joints){
				for (auto it= joints.begin(); it != joints.end(); ++it){
					if (it->get() == joint){
						joints.erase(it);
						break;
					}
				}
			}
		}
	}
	else if (currentJointType == JointType::Mirror){
		if (!objects.empty()){
			physics::Object& obj= *NONULL(objects.front());
			obj.setMirrored(!obj.isMirrored());
		}
	}
	else {
		SizeType joint_count_before= joints.size();

		switch (currentJointType){
			case JointType::Distance: {
				if (storedObjects.size() != 1 || objects.empty())
					break;

				auto joint= createJoint<physics::DistanceJoint>();
				joint->attach(	*objects.front(),
								*storedObjects[0],
								cursorOnWorld(),
								storedPosition);
			} break;
			case JointType::Prismatic: {
				if (storedObjects.size() != 2)
					break;

				util::Vec2d axis= cursorOnWorld() - storedPosition;
				auto joint= createJoint<physics::PrismaticJoint>();
				joint->attach(	*storedObjects[0],
								*storedObjects[1],
								axis);
			} break;
			case JointType::HardWeld: {
				if (storedObjects.size() != 1 || objects.empty())
					break;
				
				auto joint= createJoint<physics::HardWeldJoint>();
				joint->attach(	*storedObjects[0],
								*objects[0]);
			} break;
			case JointType::Revolute: {
				if (storedObjects.size() != 2)
					break;

				auto joint= createJoint<physics::RevoluteJoint>();
				joint->attach(	*storedObjects[0],
								*storedObjects[1],
								cursorOnWorld());
			} break;
			case JointType::Rope: {
				if (storedObjects.size() != 1 || objects.empty())
					break;

				auto joint= createJoint<physics::RopeJoint>();
				joint->attach(	*objects.front(),
								*storedObjects[0],
								cursorOnWorld(),
								storedPosition);
			} break;
			case JointType::Weld: {
				if (storedObjects.size() != 1 || objects.empty())
					break;

				auto joint= createJoint<physics::WeldJoint>();
				joint->attach(	*objects[0],
								*storedObjects[0]);
			} break;
			case JointType::Wheel: {
				if (storedObjects.size() != 2)
					break;

				util::Vec2d axis= cursorOnWorld() - storedPosition;
				auto joint= createJoint<physics::WheelJoint>();
				joint->attach(	*storedObjects[0],
								*storedObjects[1],
								axis);
			} break;
			default:;
		}
		
		if (joint_count_before == joints.size()){
			
			if (!objects.empty()){
				// No new joint
				storedObjects.pushBack(objects.front());
				storedPosition= cursorOnWorld();
			}
		}
		else {
			// Joint was created
			storedObjects.clear();
		}
	}
	
	if (objects.empty()){
		storedObjects.clear();
		return;
	}


}

void PhysicsToolsEc::onChangeJointTypeAction(int32 dir){
	int32 cur= (int32)currentJointType;
	int32 max= (int32)JointType::Last;

	currentJointType= (JointType)((cur + dir + max) % max);
}

util::Vec2d PhysicsToolsEc::cursorOnWorld() const {
	return	global::g_env->guiMgr->getCursor().getPosition().
			converted(util::Coord::World).getValue();
}

}} // game::editor
} // clover
