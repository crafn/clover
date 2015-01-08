#ifndef CLOVER_PHYSICS_JOINT_WHEEL_HPP
#define CLOVER_PHYSICS_JOINT_WHEEL_HPP

#include "b2_joint.hpp"
#include "build.hpp"

namespace clover {
namespace physics {

class Object;
class ENGINE_API WheelJoint	: public B2Joint<b2WheelJointDef, b2WheelJoint>
							, public JointTypeCrtp<WheelJoint, JointType::Wheel> {
public:
	WheelJoint();
	
	/// @param b is the wheel
	void attach(Object& a, Object& b, const WorldVec& axis);
	
	/// Attach object to the background
	void attach(Object& o, const WorldVec& axis);
	
	/// Suspension frequency
	void setSpringFrequency(real64 f);
	
	/// 0 = no damping, 1 = critical damping
	void setSpringDamping(real64 d);
	
	//
	// Motor
	//
	
	void enableMotor(bool m= true);
	bool isMotorEnabled() const;
	
	void setMaxMotorTorque(real64 torque);
	
	/// Desired angular velocity
	void setTargetAngularVelocity(real64 spd);
	real64 getTargetAngularVelocity() const;

};

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::WheelJoint> {
	static util::Str8 type(){ return "physics::WheelJoint"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_JOINT_WHEEL_HPP