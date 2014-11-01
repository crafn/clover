#include "entityobject_def.hpp"
#include "collision/shape.hpp"
#include "resources/cache.hpp"
#include "game/physicalmaterial.hpp"

namespace clover {
namespace util {

util::ObjectNode ObjectNodeTraits<physics::EntityObjectDef>::serialized(const Value& value){
	util::ObjectNode ob_node;
	ob_node["name"].setValue(value.name);
	ob_node["fixtures"].setValue(value.fixtures);
	return ob_node;
}

auto ObjectNodeTraits<physics::EntityObjectDef>::deserialized(const util::ObjectNode& ob_node) -> Value {
	Value value;
	value.name= ob_node.get("name").getValue<util::Str8>();
	value.fixtures= ob_node.get("fixtures").getValue<decltype(value.fixtures)>();
	return value;
}

util::ObjectNode ObjectNodeTraits<physics::EntityObjectDef::FixtureDef>::serialized(const Value& value){
	util::ObjectNode ob_node;
	ob_node["shape"].setValue(value.shape);
	ob_node["material"].setValue(value.material);
	return ob_node;
}

auto ObjectNodeTraits<physics::EntityObjectDef::FixtureDef>::deserialized(const util::ObjectNode& ob_node) -> Value {
	Value value;
	value.shape= ob_node.get("shape").getValue<util::Str8>();
	value.material= ob_node.get("material").getValue<util::Str8>();
	return value;
}

} // util
} // clover