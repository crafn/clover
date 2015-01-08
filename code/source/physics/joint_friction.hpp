#ifndef CLOVER_PHYSICS_JOINT_FRICTION_HPP
#define CLOVER_PHYSICS_JOINT_FRICTION_HPP

#include "build.hpp"
#include "b2_joint.hpp"

namespace clover {
namespace physics {

class Object;

/// Unused
class ENGINE_API FrictionJoint : public B2Joint<b2FrictionJointDef, b2FrictionJoint> {
public:
	void attach(Object& a, Object& b);
	
	void setMaxForce(real64 f);
	void setMaxTorque(real64 t);
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_JOINT_FRICTION_HPP
