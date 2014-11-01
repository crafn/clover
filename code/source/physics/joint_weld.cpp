#include "joint_weld.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void WeldJoint::attach(Object& a, Object& b){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	detach();
	
	addObject(a); addObject(b);
	
	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
		 
	ensure(aa && bb);
	
	util::Vec2d anchor= (a.getPosition() + b.getPosition())*0.5;
	def.Initialize(aa, bb, anchor.b2());
	
	
	if (!joint)
		createB2Joint();
		
	OnAttachCb::trigger();
}

void WeldJoint::attach(Object& a){
	attach(a, gWorld->getStaticRigidObject());
}

void WeldJoint::setFrequency(real64 f){
	def.frequencyHz= f;
	
	if (joint)
		joint->SetFrequency(f);
}


void WeldJoint::setDamping(real64 d){
	def.dampingRatio= d;
	
	if (joint)
		joint->SetDampingRatio(d);
}

} // physics
} // clover