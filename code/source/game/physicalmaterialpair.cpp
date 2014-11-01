#include "physicalmaterialpair.hpp"
#include "audio/sound.hpp"
#include "util/ensure.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace game {

PhysicalMaterialPair::PhysicalMaterialPair()
		: pair(0, 0), collisionSound(0)
		, INIT_RESOURCE_ATTRIBUTE(pairAttribute, "pair", resources::StrResourcePair("", ""))
		, INIT_RESOURCE_ATTRIBUTE(collisionSoundAttribute, "collisionSound", ""){

	collisionSoundAttribute.setOnChangeCallback([&] (){
		if (!collisionSoundAttribute.get().empty())
			collisionSound= &resources::gCache->getResource<audio::Sound>(collisionSoundAttribute.get());
		else
			collisionSound= 0;
	});
}

void PhysicalMaterialPair::setPair(const PairType& p){
	pair= p;
}

void PhysicalMaterialPair::setCollisionSound(const audio::Sound& sound){
	collisionSound= &sound;
}

const audio::Sound* PhysicalMaterialPair::getCollisionSound() const {
	return collisionSound;
}

void PhysicalMaterialPair::resourceUpdate(bool load, bool force){
	if (load){

		const auto& physmaterials_key= pairAttribute.get();

		pair= PairType(	&resources::gCache->getResource<game::PhysicalMaterial>(physmaterials_key.first),
						&resources::gCache->getResource<game::PhysicalMaterial>(physmaterials_key.second));

		util::Str8 sound_str= collisionSoundAttribute.get();

		if (!sound_str.empty())
			collisionSound= &resources::gCache->getResource<audio::Sound>(sound_str);
		else
			collisionSound= 0;

		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void PhysicalMaterialPair::createErrorResource(){
	setResourceState(State::Error);

	auto mat= resources::gCache->getErrorResource<game::PhysicalMaterial>();
	pair= PairType(&mat, &mat);
	collisionSound= &resources::gCache->getErrorResource<audio::Sound>();
}

} // game
} // clover