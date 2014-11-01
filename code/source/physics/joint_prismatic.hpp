#ifndef CLOVER_JOINT_PRISMATIC_HPP
#define CLOVER_JOINT_PRISMATIC_HPP

#include "build.hpp"
#include "b2_joint.hpp"

namespace clover {
namespace physics {

class PrismaticJoint	: public B2Joint<b2PrismaticJointDef, b2PrismaticJoint>
						, public JointTypeCrtp<	PrismaticJoint,
												JointType::Prismatic> {
public:
	struct Limit {
		Limit(real64 lower_, real64 upper_):lower(lower_), upper(upper_){}
		/// Translation
		real64 lower, upper;
	};
	
	void attach(Object& a, Object& b, const WorldVec& axis);

	/// To background
	void attach(Object& a, const WorldVec& axis);
	
	//
	// Motor
	//
	
	void enableMotor(bool m= true);
	bool isMotorEnabled() const;
	
	void setMaxMotorForce(real64 force);
	
	/// Desired velocity along axis
	void setTargetVelocity(real64 v);
	real64 getTargetVelocity() const;
	
	//
	// Limits
	//
	
	void enableLimit(bool l= true);
	bool isLimitEnabled() const;
	
	void setLimit(const Limit& l);
	Limit getLimit() const;
	
private:
};

} // physic
} // clover

#endif // CLOVER_JOINT_PRISMATIC_HPP