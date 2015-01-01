#include "box2d.hpp"
#include "joint_friction.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void FrictionJoint::attach(Object& a, Object& b){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	detach();
	
	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
	ensure(aa && bb);
	
	def.Initialize(aa, bb, toB2(a.getPosition()));

	addObject(a);
	addObject(b);
	createB2Joint();
	OnAttachCb::trigger();
}
	
void FrictionJoint::setMaxForce(real64 f){
	def.maxForce= f;
	if (joint)
		joint->SetMaxForce(f);
}

void FrictionJoint::setMaxTorque(real64 t){
	def.maxTorque= t;
	if (joint)
		joint->SetMaxTorque(t);
}

} // physics
} // clover
