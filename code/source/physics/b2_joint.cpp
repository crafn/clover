#include "b2_joint.hpp"
#include "box2d.hpp"
#include "object_rigid.hpp"

namespace clover {
namespace physics {

//
// Tightly coupled utility functions
//

template <typename T>
void createB2Joint(b2JointDef& def, T*& joint){
	ensure(!joint);
	joint= static_cast<T*>(gWorld->getB2World().CreateJoint(&def));
}

template <typename T>
void destroyB2Joint(T*& joint){
	ensure(joint);
	gWorld->getB2World().DestroyJoint(joint);
	joint= nullptr;
}

/// @note Current b2Bodies of joints can be different than new bodies in recreation
/// @todo Handle frequencies etc. when mass of attached body changes

template <typename Joint, typename Def>
void recreateB2Joint_basic(Def& def, Joint*& joint, b2Body* b1, b2Body* b2){
	ensure(b1 && b2);

	b2Vec2 new_anchor1= b1->GetLocalPoint(joint->GetAnchorA());
	b2Vec2 new_anchor2= b2->GetLocalPoint(joint->GetAnchorB());

	destroyB2Joint(joint);
	def.bodyA= b1;
	def.bodyB= b2;
	def.localAnchorA= new_anchor1;
	def.localAnchorB= new_anchor2;
	createB2Joint(def, joint);
}

template <typename Joint, typename Def>
void recreateB2Joint_refAngle(Def& def, Joint*& joint, b2Body* b1, b2Body* b2){
	ensure(b1 && b2);

	b2Vec2 new_anchor1= b1->GetLocalPoint(joint->GetAnchorA());
	b2Vec2 new_anchor2= b2->GetLocalPoint(joint->GetAnchorB());
	/// @todo Test with limits
	real64 new_ref_angle= b2->GetAngle() - b1->GetAngle();

	destroyB2Joint(joint);
	def.bodyA= b1;
	def.bodyB= b2;
	def.localAnchorA= new_anchor1;
	def.localAnchorB= new_anchor2;
	def.referenceAngle= new_ref_angle;
	createB2Joint(def, joint);
}

void recreateB2Joint(b2DistanceJointDef& def, b2DistanceJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_basic(def, joint, b1, b2);
}

void recreateB2Joint(b2FrictionJointDef& def, b2FrictionJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_basic(def, joint, b1, b2);
}

void recreateB2Joint(b2MouseJointDef& def, b2MouseJoint*& joint,
		b2Body* b1, b2Body* b2){
	ensure(b1);
	ensure(b2 == nullptr);
	
	b2Vec2 anchor= joint->GetAnchorB();
	b2Vec2 target= def.target;

	destroyB2Joint(joint);
	def.target= anchor; // Target (anchor) is in the right spot on object
	def.bodyB= b1;
	createB2Joint(def, joint);

	// Change target back to where it was before recreation (anchor remains)
	def.target= target;
	joint->SetTarget(target);
}

void recreateB2Joint(b2PrismaticJointDef& def, b2PrismaticJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_refAngle(def, joint, b1, b2);
}
	
void recreateB2Joint(b2RevoluteJointDef& def, b2RevoluteJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_refAngle(def, joint, b1, b2);
}

void recreateB2Joint(b2RopeJointDef& def, b2RopeJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_basic(def, joint, b1, b2);
}

void recreateB2Joint(b2WeldJointDef& def, b2WeldJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_refAngle(def, joint, b1, b2);
}

void recreateB2Joint(b2WheelJointDef& def, b2WheelJoint*& joint,
		b2Body* b1, b2Body* b2){
	recreateB2Joint_basic(def, joint, b1, b2);
}

//
// B2Joint
//

template <typename DefType, typename JointType>
void B2Joint<DefType, JointType>::detach(){
	Joint::detach();

	if (joint)
		destroyB2Joint();
}

template <typename DefType, typename JointType>
WorldVec B2Joint<DefType, JointType>::getAnchor(SizeType i) const {
	ensure(joint);
	if (i == 0)
		return fromB2(joint->GetAnchorA());
	if (i == 1)
		return fromB2(joint->GetAnchorB());
	release_ensure(0);
	std::abort();
}

template <typename DefType, typename JointType>
void B2Joint<DefType, JointType>::recreate(){
	ensure(isAttached());
	ensure(joint);
	
	physics::recreateB2Joint(def, joint, getB2Body(0), getB2Body(1));
}

template <typename DefType, typename JointType>
void B2Joint<DefType, JointType>::createB2Joint(){
	physics::createB2Joint(def, joint);
}

template <typename DefType, typename JointType>
void B2Joint<DefType, JointType>::destroyB2Joint(){
	physics::destroyB2Joint(joint);
}

template <typename DefType, typename JointType>
b2Body* B2Joint<DefType, JointType>::getB2Body(SizeType i) const {
	if (i < getObjectCount())
		return static_cast<RigidObject&>(getObject(i)).getB2Body();
	else
		return nullptr;
}

template class B2Joint<b2DistanceJointDef, b2DistanceJoint>;
template class B2Joint<b2FrictionJointDef, b2FrictionJoint>;
//template class B2Joint<b2GearJointDef, b2GearJoint>;
template class B2Joint<b2MouseJointDef, b2MouseJoint>;
template class B2Joint<b2PrismaticJointDef, b2PrismaticJoint>;
//template class B2Joint<b2PulleyJointDef, b2PulleyJoint>;
template class B2Joint<b2RevoluteJointDef, b2RevoluteJoint>;
template class B2Joint<b2RopeJointDef, b2RopeJoint>;
template class B2Joint<b2WeldJointDef, b2WeldJoint>;
template class B2Joint<b2WheelJointDef, b2WheelJoint>;

} // physics
} // clover
