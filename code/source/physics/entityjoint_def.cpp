#include "entityjoint_def.hpp"

namespace clover {
namespace util {

util::ObjectNode ObjectNodeTraits<physics::EntityJointDef>::serialized(const Value& value){
	util::ObjectNode ob_node;
	ob_node["type"].setValue(physics::RuntimeJointTypeTraits::enumString(value.type));
	ob_node["objects"].setValue(value.objects);
	
	if (	value.type == physics::JointType::Distance ||
			value.type == physics::JointType::Rope){	
		ob_node["anchors"].setValue(value.anchors);
	}

	if (value.type == physics::JointType::Distance){
		ob_node["frequency"].setValue(value.frequency);
		ob_node["damping"].setValue(value.damping);
	}
	
	if (value.friction != 0.0)
		ob_node["friction"].setValue(value.friction);

	return ob_node;
}

auto ObjectNodeTraits<physics::EntityJointDef>::deserialized(const util::ObjectNode& ob_node) -> Value {
	Value value;
	value.type= physics::RuntimeJointTypeTraits::jointTypeByString(ob_node.get("type").getValue<util::Str8>());
	value.objects= ob_node.get("objects").getValue<decltype(value.objects)>();
		
	if (	value.type == physics::JointType::Distance ||
			value.type == physics::JointType::Rope){	
		value.anchors= ob_node.get("anchors").getValue<decltype(value.anchors)>();
	}

	if (value.type == physics::JointType::Distance){
		value.frequency= ob_node.get("frequency").getValue<real64>();
		value.damping= ob_node.get("damping").getValue<real64>();
	}

	value.friction= ob_node.get("friction", 0.0).getValue<real64>();

	return value;
}

} // util
} // clover