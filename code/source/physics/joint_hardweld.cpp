#include "joint_hardweld.hpp"
#include "object_rigid.hpp"
#include "util/graph.hpp"

namespace clover {
namespace physics {

void HardWeldJoint::attach(Object& a, Object& b){
	attach(a, b, a.getPosition(), b.getPosition());
}

void HardWeldJoint::attach(
		Object& a, Object& b, WorldVec anchor_a, WorldVec anchor_b){
	ensure(	a.getTypeId() == Object::TypeId::Rigid && 
			b.getTypeId() == Object::TypeId::Rigid);
	
	detach();
	
	addObject(a);
	addObject(b);
	localAnchors[0]= anchor_a*a.getTransform().inversed();
	localAnchors[1]= anchor_b*b.getTransform().inversed();

	getRigidObject(1).weld(getRigidObject(0));
	attached= true;

	OnAttachCb::trigger();

}

void HardWeldJoint::detach(){
	if (isAttached()){
		// Welding of objects doesn't recognize the joint hierarchy.
		// In order to have symmetry in attaching and detaching
		// object groups, detaching a joint is done the following way:
		//
		// 1. Joints of a weld group are searched
		// 2. All welds are removed
		// 3. Every weld is redone, except the one beloning to 'this'

		auto joints= util::findGraphNodes<util::Set<HardWeldJoint*>, HardWeldJoint*>(
			{ this },
			[] (HardWeldJoint* node) -> util::Set<HardWeldJoint*> {
				return node->getNeighborHardWeldJoints();
			}
		);

		getRigidObject(1).unweldGroup();

		for (HardWeldJoint* joint : joints){
			if (joint == this)
				continue;

			joint->getRigidObject(1).weld(joint->getRigidObject(0));
		}

		attached= false;
	}

	Base::detach();
}

WorldVec HardWeldJoint::getAnchor(SizeType i) const {
	ensure(i < 2);
	return localAnchors[i]*getObject(i).getTransform();
}

RigidObject& HardWeldJoint::getRigidObject(SizeType i) const {
	return static_cast<RigidObject&>(Base::getObject(i));
} 

util::Set<HardWeldJoint*> HardWeldJoint::getNeighborHardWeldJoints() const {
	util::Set<HardWeldJoint*> connected;
	for (SizeType i= 0; i < getObjectCount(); ++i){
		for (Joint* j : getRigidObject(i).getJoints()){
			auto ptr= dynamic_cast<HardWeldJoint*>(j);
			if (ptr)
				connected.insert(ptr);
		}
	}
	return connected;
}

} // physics
} // clover