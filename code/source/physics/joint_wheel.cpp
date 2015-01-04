#include "box2d.hpp"
#include "joint_wheel.hpp"
#include "object_rigid.hpp"
#include "phys_mgr.hpp"

namespace clover {
namespace physics {

WheelJoint::WheelJoint(){
	def.frequencyHz= 10.0;
}

void WheelJoint::attach(Object& a, Object& b, const WorldVec& axis){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
	ensure(aa && bb);
	
	detach();
	
	addObject(a);
	addObject(b);
	
	util::Vec2d anchor= b.getPosition();
	def.Initialize(aa, bb, toB2(anchor), toB2(axis));
	
	if (!joint)
		createB2Joint();
	
	OnAttachCb::trigger();
}

void WheelJoint::attach(Object& o, const WorldVec& axis){
	attach(o, global::g_env->physMgr->getWorld().getStaticRigidObject(), axis);
}

void WheelJoint::setSpringFrequency(real64 f){
	def.frequencyHz= f;
	
	if (joint)
		joint->SetSpringFrequencyHz(f);
}

void WheelJoint::setSpringDamping(real64 d){
	def.dampingRatio= d;
	
	if (joint)
		joint->SetSpringDampingRatio(d);
}

void WheelJoint::enableMotor(bool m){
	def.enableMotor= m;
	if (joint)
		joint->EnableMotor(m);
}

bool WheelJoint::isMotorEnabled() const {
	return def.enableMotor;
}

void WheelJoint::setMaxMotorTorque(real64 torque){
	def.maxMotorTorque= torque;
	if (joint)
		joint->SetMaxMotorTorque(torque);
}

void WheelJoint::setTargetAngularVelocity(real64 a){
	def.motorSpeed= a;
	if (joint)
		joint->SetMotorSpeed(a);
}

real64 WheelJoint::getTargetAngularVelocity() const {
	return def.motorSpeed;
}

} // physics
} // clover
