#include "audio/sound.hpp"
#include "global/env.hpp"
#include "physicalmaterialpair.hpp"
#include "resources/cache.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace game {

PhysicalMaterialPair::PhysicalMaterialPair()
		: pair(0, 0), collisionSound(0)
		, INIT_RESOURCE_ATTRIBUTE(pairAttribute, "pair", resources::StrResourcePair("", ""))
		, INIT_RESOURCE_ATTRIBUTE(collisionSoundAttribute, "collisionSound", ""){

	collisionSoundAttribute.setOnChangeCallback([&] (){
		if (!collisionSoundAttribute.get().empty())
			collisionSound= &global::g_env->resCache->getResource<audio::Sound>(collisionSoundAttribute.get());
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

		pair= PairType(	&global::g_env->resCache->getResource<game::PhysicalMaterial>(physmaterials_key.first),
						&global::g_env->resCache->getResource<game::PhysicalMaterial>(physmaterials_key.second));

		util::Str8 sound_str= collisionSoundAttribute.get();

		if (!sound_str.empty())
			collisionSound= &global::g_env->resCache->getResource<audio::Sound>(sound_str);
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

	auto mat= global::g_env->resCache->getErrorResource<game::PhysicalMaterial>();
	pair= PairType(&mat, &mat);
	collisionSound= &global::g_env->resCache->getErrorResource<audio::Sound>();
}

} // game
} // clover
