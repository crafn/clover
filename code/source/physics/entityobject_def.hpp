#ifndef CLOVER_PHYSICS_ENTITYOBJECT_DEF_HPP
#define CLOVER_PHYSICS_ENTITYOBJECT_DEF_HPP

#include "build.hpp"
#include "util/objectnodetraits.hpp"

namespace clover {
namespace physics {

class Material;

/// Helper class for EntityDef
struct EntityObjectDef {
	struct FixtureDef {
		util::Str8 shape;
		util::Str8 material;
	};
	
	util::Str8 name;
	util::DynArray<FixtureDef> fixtures;
};

} // physics
namespace util {

template <>
struct ObjectNodeTraits<physics::EntityObjectDef> {
	typedef physics::EntityObjectDef Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

template <>
struct ObjectNodeTraits<physics::EntityObjectDef::FixtureDef> {
	typedef physics::EntityObjectDef::FixtureDef Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_PHYSICS_ENTITYOBJECT_DEF_HPP
