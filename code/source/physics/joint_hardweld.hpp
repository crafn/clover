#ifndef CLOVER_PHYSICS_JOINT_HARDWELD_HPP
#define CLOVER_PHYSICS_JOINT_HARDWELD_HPP

#include "build.hpp"
#include "joint.hpp"
#include "util/set.hpp"
#include "util/vector.hpp"

namespace clover {
namespace physics {

class RigidObject;

/// Joins two objects to form a single rigid body
class ENGINE_API HardWeldJoint	: public Joint
								, public JointTypeCrtp<	HardWeldJoint,
														JointType::HardWeld> {
public:
	using Base= Joint;

	virtual ~HardWeldJoint(){ detach(); }

	void attach(Object& this_to, Object& that);
	void attach(Object& this_to, Object& that, WorldVec a1, WorldVec a2);
	virtual void detach() override;
	virtual bool isAttached() const { return attached; }

	virtual WorldVec getAnchor(SizeType i) const override;

private:
	RigidObject& getRigidObject(SizeType i) const;
	util::Set<HardWeldJoint*> getNeighborHardWeldJoints() const;
	virtual void recreate(){}
	
	bool attached= false;
	util::Vec2d localAnchors[2];
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_JOINT_HARDWELD_HPP
