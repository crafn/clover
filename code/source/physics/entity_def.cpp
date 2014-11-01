#include "entity_def.hpp"
#include "animation/armature.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace physics {

EntityDef::EntityDef()
	: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
	, INIT_RESOURCE_ATTRIBUTE(armatureAttribute, "armature", "")
	, INIT_RESOURCE_ATTRIBUTE(objectDefsAttribute, "objects", {})
	, INIT_RESOURCE_ATTRIBUTE(jointDefsAttribute, "joints", {}){
}

void EntityDef::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		/// @todo Check that object names match to armature joints
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void EntityDef::createErrorResource(){
	setResourceState(State::Error);
}

const animation::Armature& EntityDef::getArmature() const {
	return resources::gCache->getResource<animation::Armature>(armatureAttribute.get());
}

const util::DynArray<EntityObjectDef>& EntityDef::getObjectDefs() const {
	return objectDefsAttribute.get();
}

const util::DynArray<EntityJointDef>& EntityDef::getJointDefs() const {
	return jointDefsAttribute.get();
}

} // physics
} // clover