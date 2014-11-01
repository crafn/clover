#include "joint_target.hpp"
#include "world.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void TargetJoint::attach(Object& o, util::Vec2d world_anchor_pos){
	ensure(o.getTypeId() == Object::TypeId::Rigid);

	detach();

	addObject(o);
	def.bodyA= gWorld->getStaticRigidObject().getB2Body();
	def.bodyB= static_cast<RigidObject*>(&o)->getB2Body();
	def.target= world_anchor_pos.b2();

	if (!joint)
		createB2Joint();
		
	OnAttachCb::trigger();
}

void TargetJoint::setMaxForce(real64 force){
	def.maxForce= force;
	if (joint)
		joint->SetMaxForce(force);
}

void TargetJoint::setFrequency(real64 f){
	def.frequencyHz= f;
	if (joint)
		joint->SetFrequency(f);
}

void TargetJoint::setDamping(real64 d){
	def.dampingRatio= d;
	if (joint)
		joint->SetDampingRatio(d);
}

void TargetJoint::setTarget(util::Vec2d t){
	def.target= t.b2();
	if (joint)
		joint->SetTarget(t.b2());
}

} // physics
} // clover