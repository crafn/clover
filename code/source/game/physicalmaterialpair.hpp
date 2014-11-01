#ifndef CLOVER_GAME_PHYSICALMATERIALPAIR_HPP
#define CLOVER_GAME_PHYSICALMATERIALPAIR_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "resources/resourcepair.hpp"
#include "physicalmaterial.hpp"

#include <utility>

namespace clover {
namespace game {
class PhysicalMaterialPair;
} // game

namespace resources {

template<>
struct ResourceTraits<game::PhysicalMaterialPair> {
	DECLARE_RESOURCE_TRAITS(game::PhysicalMaterialPair, ResourcePair);
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::ResourcePair("pair"),
							AttributeDef::Resource("collisionSound", "Audio"))
	
	typedef SubCache<game::PhysicalMaterialPair> SubCacheType;

	static util::Str8 typeName(){ return "PhysMaterialPair"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return util::Str8::format("%s %s", key.first.cStr(), key.second.cStr()); }
	static util::Str8 identifierKey(){ return "pair"; }


	static bool expired(const game::PhysicalMaterialPair&){ return false; }
};

} // resources
namespace game {

using resources::AttributeType;
using resources::Attribute;

/// Physics material pair
class PhysicalMaterialPair : public resources::Resource {
public:
	DECLARE_RESOURCE(PhysicalMaterialPair)
	
	typedef std::pair<const game::PhysicalMaterial*, const game::PhysicalMaterial*> PairType;
	typedef resources::StrResourcePair IdentifierValue;

	PhysicalMaterialPair();

	void setPair(const PairType& p);
	const PairType& getPair() const { return pair; }

	void setCollisionSound(const audio::Sound& sound);
	const audio::Sound* getCollisionSound() const;

	virtual void resourceUpdate(bool load, bool force=true);
	virtual void createErrorResource();

private:
	PairType pair;
	const audio::Sound* collisionSound;
	
	RESOURCE_ATTRIBUTE(ResourcePair, pairAttribute);
	RESOURCE_ATTRIBUTE(Resource, collisionSoundAttribute);
};

} // game
} // clover

#endif // CLOVER_GAME_PHYSICALMATERIALPAIR_HPP