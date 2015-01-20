#ifndef CLOVER_PHYSICS_JOINT_REVOLUTE_HPP
#define CLOVER_PHYSICS_JOINT_REVOLUTE_HPP

#include "b2_joint.hpp"
#include "build.hpp"

namespace clover {
namespace physics {

class Object;

class RevoluteJoint : public B2Joint<b2RevoluteJointDef, b2RevoluteJoint>
					, public JointTypeCrtp<	RevoluteJoint,
											JointType::Revolute> {
public:
	
	/// Attach two objects together
	void attach(Object& a, Object& b, const WorldVec& anchor_pos);
	
	/// Attach object to the background
	void attach(Object& o, const WorldVec& anchor_pos);
	
	//
	// Motor
	//
	
	void enableMotor(bool m= true);
	bool isMotorEnabled() const;
	
	void setMaxMotorTorque(real64 torque);
	
	/// Desired angular velocity
	void setTargetAngularVelocity(real64 spd);
	real64 getTargetAngularVelocity() const;
	
	//
	// Limits
	//
	
	void enableLimit(bool l= true);
	bool isLimitEnabled() const;
	
	struct Limit {
		real64 lower, upper;
	};
	
	void setLimit(const Limit& l);
	Limit getLimit() const;
	
	
	
private:
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_JOINT_REVOLUTE_HPP
