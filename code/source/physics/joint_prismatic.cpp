#include "box2d.hpp"
#include "joint_prismatic.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void PrismaticJoint::attach(Object& a, Object& b, const WorldVec& axis){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	detach();

	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
	ensure(aa && bb);
	
	addObject(a); addObject(b);

	util::Vec2d anchor= (a.getPosition() + b.getPosition())*0.5;
	def.Initialize(aa, bb, toB2(anchor), toB2(axis));
	if (!joint)
		createB2Joint();
		
	OnAttachCb::trigger();
}

void PrismaticJoint::attach(Object& o, const WorldVec& axis){
	attach(o, gWorld->getStaticRigidObject(), axis);
}	

void PrismaticJoint::enableMotor(bool m){
	def.enableMotor= m;
	if (joint)
		joint->EnableMotor(m);
}

bool PrismaticJoint::isMotorEnabled() const {
	return def.enableMotor;
}

void PrismaticJoint::setMaxMotorForce(real64 force){
	def.maxMotorForce= force;
	if (joint)
		joint->SetMaxMotorForce(force);
}

void PrismaticJoint::setTargetVelocity(real64 v){
	def.motorSpeed= v;
	if (joint)
		joint->SetMotorSpeed(v);
}

real64 PrismaticJoint::getTargetVelocity() const {
	return def.motorSpeed;
}

void PrismaticJoint::enableLimit(bool l){
	def.enableLimit= l;
	if (joint)
		joint->EnableLimit(l);
}

bool PrismaticJoint::isLimitEnabled() const{
	return def.enableLimit;
}


PrismaticJoint::Limit PrismaticJoint::getLimit() const {
	return Limit { def.lowerTranslation, def.upperTranslation };
}

void PrismaticJoint::setLimit(const Limit& l){
	def.lowerTranslation= l.lower;
	def.upperTranslation= l.upper;
	
	if (joint)
		joint->SetLimits(l.lower, l.upper);
}

} // physics
} // clover
