#include "joint.hpp"

namespace clover {
namespace animation {

Joint::Joint(const util::Str8& name_, const JointPose& bind_pose_, JointId id_, JointId super_id_)
		: bindPose(bind_pose_)
		, name(name_)
		, id(id_)
		, superId(super_id_){
}

} // animation
} // clover