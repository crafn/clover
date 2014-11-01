#ifndef CLOVER_PHYSICS_ENTITYJOINT_DEF_HPP
#define CLOVER_PHYSICS_ENTITYJOINT_DEF_HPP

#include "build.hpp"
#include "jointtype.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace physics {
	
/// Helper class for EntityDef
/// @todo Limits and motors
struct EntityJointDef {
	JointType type= JointType::None;
	util::DynArray<util::Str8> objects;
	util::DynArray<util::Vec2d> anchors;
	real64 frequency= 1.0;
	real64 damping= 0.5;
	real64 friction= 0.0;
};

} // physics
namespace util {

template <>
struct ObjectNodeTraits<physics::EntityJointDef> {
	typedef physics::EntityJointDef Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_PHYSICS_ENTITYJOINT_DEF_HPP
