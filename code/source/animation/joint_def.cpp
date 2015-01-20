#include "joint_def.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace util {

util::ObjectNode ObjectNodeTraits<animation::JointDef>::serialized(const Value& value){
	util::ObjectNode ob_node;
	ob_node["name"].setValue(value.name);
	ob_node["superJoint"].setValue(value.superJointName);
	ob_node["transform"].setValue(value.transform);
	return ob_node;
}

auto ObjectNodeTraits<animation::JointDef>::deserialized(const util::ObjectNode& ob_node) -> Value {
	Value value;
	value.name= ob_node.get("name").getValue<util::Str8>();
	value.superJointName= ob_node.get("superJoint").getValue<util::Str8>();
	value.transform= ob_node.get("transform").getValue<animation::JointPose::Transform>();
	return value;
}

} // util
} // clover
