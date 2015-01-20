#ifndef CLOVER_PHYSICS_JOINT_WELD_HPP
#define CLOVER_PHYSICS_JOINT_WELD_HPP

#include "b2_joint.hpp"
#include "build.hpp"
#include "util/traits.hpp"

namespace clover {
namespace physics {

class Object;

class ENGINE_API WeldJoint : public B2Joint<b2WeldJointDef, b2WeldJoint>
							, public JointTypeCrtp<WeldJoint, JointType::Weld> {
public:
	void attach(Object& a, Object& b);

	/// To background
	void attach(Object& a);
	
	void setFrequency(real64 f);
	
	/// 0 = no damping, 1 = critical damping
	void setDamping(real64 d);
	

private:
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_JOINT_WELD_HPP
