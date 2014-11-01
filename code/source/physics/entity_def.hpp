#ifndef CLOVER_PHYSICS_ENTITY_DEF
#define CLOVER_PHYSICS_ENTITY_DEF

#include "build.hpp"
#include "entityobject_def.hpp"
#include "entityjoint_def.hpp"
#include "resources/resource.hpp"
#include "util/class_preproc.hpp"

namespace clover {
namespace animation {

class Armature;

} // animation
namespace physics {

class EntityDef;

} // physics
namespace resources {

template <>
struct ResourceTraits<physics::EntityDef> {
	DECLARE_RESOURCE_TRAITS(physics::EntityDef, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"))
	
	typedef SubCache<physics::EntityDef> SubCacheType;
	
	static util::Str8 typeName(){ return "PhysEntityDef"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& value){ return value; }
	static util::Str8 identifierKey(){ return "name"; }
	static bool expired(const physics::EntityDef&){ return false; }
};

} // resources
namespace physics {

class EntityDef : public resources::Resource {
public:
	DECLARE_RESOURCE(EntityDef);

	EntityDef();
	DELETE_COPY(EntityDef);
	DELETE_MOVE(EntityDef);

	const util::Str8& getName() const { return nameAttribute.get(); }

	virtual void resourceUpdate(bool load, bool force= true) override;
	virtual void createErrorResource() override;

	const animation::Armature& getArmature() const;
	const util::DynArray<EntityObjectDef>& getObjectDefs() const;
	const util::DynArray<EntityJointDef>& getJointDefs() const;

private:
	RESOURCE_ATTRIBUTE(String, nameAttribute);
	RESOURCE_ATTRIBUTE(Resource, armatureAttribute);
	RESOURCE_ATTRIBUTE(PhysObjectDefArray, objectDefsAttribute);
	RESOURCE_ATTRIBUTE(PhysJointDefArray, jointDefsAttribute);

};

} // physics
namespace util {

template <>
struct TypeStringTraits<physics::EntityDef> {
	static util::Str8 type(){ return "physics::EntityDef"; }
};

} // util
} // clover

#endif // CLOVER_PHYSICS_ENTITY_DEF