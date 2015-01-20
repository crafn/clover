#ifndef CLOVER_PHYSICS_JOINT_ROPE_HPP
#define CLOVER_PHYSICS_JOINT_ROPE_HPP

#include "build.hpp"
#include "b2_joint.hpp"

namespace clover {
namespace physics {

class ENGINE_API RopeJoint : public B2Joint<b2RopeJointDef, b2RopeJoint>
							, public JointTypeCrtp<RopeJoint, JointType::Rope> {
public:
	void attach(Object& a, Object& b,
				const WorldVec& anchor_a, const WorldVec& anchor_b);

	/// Sets the maximum distance between anchors
	void setLength(real64 d);
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_JOINT_ROPE_HPP
