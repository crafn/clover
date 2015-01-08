#ifndef PHYSICS_JOINT_TARGET_HPP
#define PHYSICS_JOINT_TARGET_HPP

#include "build.hpp"
#include "b2_joint.hpp"
#include "util/vector.hpp"

#include <Box2D/Box2D.h>

namespace clover {
namespace physics {

/// A springlike joint between a point and object
/// @warning Use only for testing, lacks functionality of normal joints
class ENGINE_API TargetJoint : public B2Joint<b2MouseJointDef, b2MouseJoint> {
public:
	void attach(Object& o, WorldVec world_anchor_pos);

	void setMaxForce(real64 force);
	void setFrequency(real64 f);
	void setDamping(real64 d);
	void setTarget(util::Vec2d t);
};

} // physics
} // clover

#endif // PHYSICS_JOINT_TARGET_HPP
