#include "box2d.hpp"
#include "joint_revolute.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void RevoluteJoint::attach(Object& a, Object& b, const WorldVec& anchor_pos){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
			 
	ensure(aa && bb);
	
	detach();
	
	addObject(a); addObject(b);
	
	def.Initialize(	aa, 
					bb,
					toB2(anchor_pos));
	
	if (!joint)
		createB2Joint();
		
	OnAttachCb::trigger();
}

void RevoluteJoint::attach(Object& o, const WorldVec& anchor_pos){
	attach(o, gWorld->getStaticRigidObject(), anchor_pos);
}	

void RevoluteJoint::enableMotor(bool m){
	def.enableMotor= m;
	if (joint)
		joint->EnableMotor(m);
}

bool RevoluteJoint::isMotorEnabled() const {
	return def.enableMotor;
}

void RevoluteJoint::setMaxMotorTorque(real64 torque){
	def.maxMotorTorque= torque;
	if (joint)
		joint->SetMaxMotorTorque(torque);
}

void RevoluteJoint::setTargetAngularVelocity(real64 a){
	def.motorSpeed= a;
	if (joint)
		joint->SetMotorSpeed(a);
}

real64 RevoluteJoint::getTargetAngularVelocity() const {
	return def.motorSpeed;
}

void RevoluteJoint::enableLimit(bool l){
	def.enableLimit= l;
	if (joint)
		joint->EnableLimit(l);
}

bool RevoluteJoint::isLimitEnabled() const{
	return def.enableLimit;
}

RevoluteJoint::Limit RevoluteJoint::getLimit() const {
	return Limit { def.lowerAngle, def.upperAngle };
}

void RevoluteJoint::setLimit(const Limit& l){
	def.lowerAngle= l.lower;
	def.upperAngle= l.upper;
	
	if (joint)
		joint->SetLimits(l.lower, l.upper);
}

} // physics
} // clover
