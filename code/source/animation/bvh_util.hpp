#ifndef CLOVER_ANIMATION_BVH_UTIL_HPP
#define CLOVER_ANIMATION_BVH_UTIL_HPP

#include "armaturepose.hpp"
#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/hashmap.hpp"
#include "util/math.hpp"
#include "util/string.hpp"

namespace clover {
namespace animation {
	
/// Defines a joint in bvh format
struct BvhJoint {
	enum class Channel {
		PosX,
		PosY,
		PosZ,
		RotX,
		RotY,
		RotZ
	};
	
	util::Str8 name;
	util::Vec3f offset;
	util::DynArray<Channel> channels;
	util::DynArray<BvhJoint> subJoints;
};

/// Reflects bvh format
struct BvhData {
	BvhJoint rootJoint;
	SizeType frameCount;
	real32 frameTime;
	util::DynArray<real32> samples;
};

BvhData parseBvhAnimation(const util::Str8& data);

/// @return Local poses (not in-bind)
util::DynArray<ArmaturePose::Pose> calcLocalPosesFromBvh(const BvhData& bvh_data, const util::HashMap<util::Str8, JointId>& joint_name_to_id);

} // animation
} // clover

#endif // CLOVER_ANIMATION_BVH_UTIL_HPP