#include "compositionnodeslottemplategroup.hpp"

namespace clover {
namespace nodes {

SlotIdentifier SlotTemplate::getIdentifier() const {
	return SlotIdentifier{name, owner->getName(), signalType, input};
}

CompositionNodeSlotTemplateGroup::CompositionNodeSlotTemplateGroup(CompositionNodeLogic& owner_, const util::Str8& name_, bool is_input)
		: owner(&owner_)
		, name(name_)
		, inputType(is_input)
		, variant(false)
		, mirror(false){}


void CompositionNodeSlotTemplateGroup::addSlotTemplate(const util::Str8& name, SignalType signaltype){
	slots.pushBack(SlotTemplate(*this, name, signaltype, inputType));
}

SlotTemplate CompositionNodeSlotTemplateGroup::getSlotTemplate(const SlotIdentifier& id){
	if (isVariant())
		return SlotTemplate(*this, id.name, id.signalType, id.input);
		
	for (auto& m : slots){
		if (m.getIdentifier() == id) return m;
	}
	
	throw global::Exception("SlotTemplate not found: %s", id.getString().cStr());
}

bool CompositionNodeSlotTemplateGroup::hasSlotTemplate(const SlotIdentifier& id) const {
	for (auto& m : slots){
		if (m.getIdentifier() == id) return true;
	}
	return false;
}

} // nodes
} // clover