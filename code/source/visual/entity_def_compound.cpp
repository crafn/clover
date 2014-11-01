#include "entity_def_compound.hpp"
#include "entitylogic_compound.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace visual {

CompoundEntityDef::CompoundEntityDef()
		: INIT_RESOURCE_ATTRIBUTE(armatureAttribute, "armature", "")
		, INIT_RESOURCE_ATTRIBUTE(attachmentsAttribute, "attachments", {}){
	type= Type::Compound;
	
	armatureAttribute.setOnChangeCallback([this] (){
		if (getResourceState() != State::Uninit)
			resourceUpdate(true);
	});
}

const animation::Armature& CompoundEntityDef::getArmature() const {
	return resources::gCache->getResource<animation::Armature>(armatureAttribute.get());
}

const util::DynArray<ArmatureAttachmentDef>& CompoundEntityDef::getAttachmentDefs() const {
	return attachmentsAttribute.get();
}

CompoundEntityLogic* CompoundEntityDef::createLogic() const {
	return new CompoundEntityLogic(*this);
}

void CompoundEntityDef::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		armatureChangeListener.clear();
		armatureChangeListener.listen(getArmature(), [this] (){
			OnChangeCb::trigger();
		});
		
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void CompoundEntityDef::createErrorResource(){
	/// @todo
	setResourceState(State::Error);
}

} // visual
} // clover