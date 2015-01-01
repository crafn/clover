#ifndef CLOVER_GAME_PHYSICALMATERIAL_HPP
#define CLOVER_GAME_PHYSICALMATERIAL_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "physics/material.hpp"

namespace clover {
namespace audio {

class Sound;

} // audio
namespace game {

class PhysicalMaterial;

} // game

using resources::AttributeType;
using resources::Attribute;

namespace resources {

template <>
struct ResourceTraits<game::PhysicalMaterial> {
	DECLARE_RESOURCE_TRAITS(game::PhysicalMaterial, String);
	
	typedef SubCache<game::PhysicalMaterial> SubCacheType;
	
	static util::Str8 typeName(){ return "PhysMaterial"; }

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Real("density", false, 1.0),
							AttributeDef::Real("friction", false, 0),
							AttributeDef::Real("restitution", false, 0, 1),
							AttributeDef::Real("toughness", false, -1.0),
							AttributeDef::Resource("defaultCollisionSound", "Audio"),
							AttributeDef::Boolean("allowCollisionSound"))
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static util::Str8 identifierKey(){ return "name"; }


	static bool expired(const game::PhysicalMaterial&){ return false; }
};

} // resources
namespace game {

class PhysicalMaterial : public resources::Resource, public physics::Material {
public:
	DECLARE_RESOURCE(PhysicalMaterial)
	
	PhysicalMaterial();

	const audio::Sound* getDefaultCollisionSound() const { return defaultCollisionSound; }
	bool isAllowingCollisionSound() const { return allowCollisionSoundAttribute.get(); }

	const util::Str8& getName() const { return nameAttribute.get(); }

	virtual void resourceUpdate(bool load, bool force=true);
	virtual void createErrorResource();
	
private:
	const audio::Sound* defaultCollisionSound;
	
	void setDefaultCollisionSound(const util::Str8& name);
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Real, densityAttribute)
	RESOURCE_ATTRIBUTE(Real, frictionAttribute)
	RESOURCE_ATTRIBUTE(Real, restitutionAttribute)
	RESOURCE_ATTRIBUTE(Real, toughnessAttribute)
	RESOURCE_ATTRIBUTE(Resource, defaultCollisionSoundAttribute)
	RESOURCE_ATTRIBUTE(Boolean, allowCollisionSoundAttribute)
};

} // game
namespace util {

template <>
struct TypeStringTraits<game::PhysicalMaterial> {
	static util::Str8 type(){ return "PhysicalMaterial"; }
};

} // util
} // clover

#endif // CLOVER_GAME_PHYSICALMATERIAL_HPP
