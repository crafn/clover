#include "box2d.hpp"
#include "joint_distance.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

void DistanceJoint::attach(	Object& a, Object& b, 
							const WorldVec& anchor_a, const WorldVec& anchor_b){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	detach();
	
	addObject(a); addObject(b);
	
	b2Body* aa= static_cast<RigidObject*>(&a)->getB2Body(),
		  * bb= static_cast<RigidObject*>(&b)->getB2Body();
	ensure(aa && bb);
	
	def.Initialize(aa, bb, toB2(anchor_a), toB2(anchor_b));
	
	if (!joint)
		createB2Joint();
		
	OnAttachCb::trigger();
}

void DistanceJoint::attach(Object& a,
						   const WorldVec& anchor_a, const WorldVec& anchor_b){
	attach(a, gWorld->getStaticRigidObject(), anchor_a, anchor_b);
}

void DistanceJoint::setDistance(real64 d){
	def.length= d;
	if (joint)
		joint->SetLength(d);
}

void DistanceJoint::setFrequency(real64 f){
	def.frequencyHz= f;
	if (joint)
		joint->SetFrequency(f);
}


void DistanceJoint::setDamping(real64 d){
	def.dampingRatio= d;
	if (joint)
		joint->SetDampingRatio(d);
}

} // physics
} // clover
