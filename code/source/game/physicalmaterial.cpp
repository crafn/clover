#include "audio/sound.hpp"
#include "global/env.hpp"
#include "physicalmaterial.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace game {

PhysicalMaterial::PhysicalMaterial():
	defaultCollisionSound(0),
	INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", ""),
	INIT_RESOURCE_ATTRIBUTE(densityAttribute, "density", getDensity()),
	INIT_RESOURCE_ATTRIBUTE(frictionAttribute, "friction", getFriction()),
	INIT_RESOURCE_ATTRIBUTE(restitutionAttribute, "restitution", getRestitution()),
	INIT_RESOURCE_ATTRIBUTE(toughnessAttribute, "toughness", getToughness()),
	INIT_RESOURCE_ATTRIBUTE(defaultCollisionSoundAttribute, "defaultCollisionSound", util::Str8("")),
	INIT_RESOURCE_ATTRIBUTE(allowCollisionSoundAttribute, "allowCollisionSound", true){

	densityAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::Phys, debug::Vb::Trivial, "Density changed for %s: %f",
				nameAttribute.get().cStr(), densityAttribute.get());
			setDensity(densityAttribute.get());
		}
	});
	
	frictionAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::Phys, debug::Vb::Trivial, "Friction changed for %s: %f",
				nameAttribute.get().cStr(), frictionAttribute.get());
			setFriction(frictionAttribute.get());
		}
	});
	
	restitutionAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::Phys, debug::Vb::Trivial, "Restitution changed for %s: %f",
				nameAttribute.get().cStr(), restitutionAttribute.get());
			setRestitution(restitutionAttribute.get());
		}
	});
		
	toughnessAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::Phys, debug::Vb::Trivial, "Toughness changed for %s: %f",
				nameAttribute.get().cStr(), toughnessAttribute.get());
			setToughness(toughnessAttribute.get());
		}
	});
	
	defaultCollisionSoundAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::Phys, debug::Vb::Trivial, "Collision sound changed for %s: %s",
				nameAttribute.get().cStr(), defaultCollisionSoundAttribute.get().cStr());
			setDefaultCollisionSound(defaultCollisionSoundAttribute.get());
		}
	});
}

void PhysicalMaterial::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){

		setDensity(densityAttribute.get());
		setFriction(frictionAttribute.get());
		setRestitution(restitutionAttribute.get());
		setToughness(toughnessAttribute.get());

		setDefaultCollisionSound(defaultCollisionSoundAttribute.get());

		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void PhysicalMaterial::createErrorResource(){
	setResourceState(State::Error);

	setDensity(1.0);
	setFriction(0.5);
	setRestitution(0.5);
	setToughness(-1.0);
}

void PhysicalMaterial::setDefaultCollisionSound(const util::Str8& sound_name){
		if (!sound_name.empty()){
			defaultCollisionSound= &global::g_env.resCache->getResource<audio::Sound>(sound_name);
		}
		else
			defaultCollisionSound= 0;
}

} // game
} // clover
