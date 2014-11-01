#include "joint_rope.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void RopeJoint::attach(	Object& a, Object& b,
						const WorldVec& anchor_a, const WorldVec& anchor_b){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
	ensure(aa && bb);

	detach();
	addObject(a);
	addObject(b);

	def.bodyA= aa;
	def.bodyB= bb;
	def.localAnchorA= aa->GetLocalPoint(anchor_a.b2());
	def.localAnchorB= bb->GetLocalPoint(anchor_b.b2());
	def.maxLength= (anchor_b - anchor_a).length();
	createB2Joint();
		
	OnAttachCb::trigger();
}

void RopeJoint::setLength(real64 d){
	def.maxLength= d;
	if (joint)
		joint->SetMaxLength(d);
}

} // physics
} // clover