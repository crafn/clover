#ifndef CLOVER_ANIMATION_JOINT_DEF_HPP
#define CLOVER_ANIMATION_JOINT_DEF_HPP

#include "build.hpp"
#include "jointpose.hpp"
#include "util/objectnode.hpp"
#include "util/string.hpp"

namespace clover {
namespace animation {

/// Resource attribute which defines joint of Armature
struct JointDef {
	util::Str8 name;
	util::Str8 superJointName;
	JointPose::Transform transform;
};

} // animation
namespace util {

template <>
struct ObjectNodeTraits<animation::JointDef> {
	typedef animation::JointDef Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_ANIMATION_JOINT_DEF_HPP