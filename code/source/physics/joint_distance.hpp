#ifndef CLOVER_JOINT_DISTANCE_HPP
#define CLOVER_JOINT_DISTANCE_HPP

#include "build.hpp"
#include "b2_joint.hpp"

namespace clover {
namespace physics {

class Object;

class DistanceJoint : public B2Joint<b2DistanceJointDef, b2DistanceJoint>
					, public JointTypeCrtp<	DistanceJoint,
											JointType::Distance> {
public:
	void attach(Object& a, Object& b,
				const WorldVec& anchor_a, const WorldVec& anchor_b);
	
	/// To background
	void attach(Object& a,
				const WorldVec& anchor_a, const WorldVec& anchor_background);
	
	/// Sets the distance between anchors manually (use carefully)
	void setDistance(real64 d);
	
	void setFrequency(real64 f);
	
	/// 0 = no damping, 1 = critical damping
	void setDamping(real64 d);
	
};

} // physics
} // clover

#endif // CLOVER_JOINT_DISTANCE_HPP
