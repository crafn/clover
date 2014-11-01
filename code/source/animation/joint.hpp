#ifndef CLOVER_ANIMATION_JOINT_HPP
#define CLOVER_ANIMATION_JOINT_HPP

#include "build.hpp"
#include "jointpose.hpp"
#include "util/string.hpp"

namespace clover {
namespace animation {

/// Joint of Armature
class Joint {
public:
	Joint(const util::Str8& name, const JointPose& bind_pose, JointId id, JointId super_id= JointIdNone);
	
	const util::Str8& getName() const { return name; }
	JointId getId() const { return id; }
	JointId getSuperId() const { return superId; }
	const JointPose& getBindPose() const { return bindPose; }
	
private:
	JointPose bindPose;
	
	util::Str8 name;
	JointId id;
	JointId superId;
};

} // animation
} // clover

#endif // CLOVER_ANIMATION_JOINT_HPP