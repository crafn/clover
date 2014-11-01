#include "compositionnodeslothandle.hpp"
#include "compositionnodelogic.hpp"
#include "compositionnodeslot.hpp"

namespace clover {
namespace nodes {

CompositionNodeSlotHandle::CompositionNodeSlotHandle(CompositionNodeSlot& slot)
		: owner(&slot.getOwner())
		, identifier(slot.getIdentifier()){
	ensure(owner);	
}

CompositionNodeSlotHandle::operator bool() const {
	return owner->hasSlot(identifier);
}

CompositionNodeSlot* CompositionNodeSlotHandle::get() const {
	if (*this)
		return &owner->getSlot(identifier);
	return nullptr;
}

CompositionNodeSlot* CompositionNodeSlotHandle::operator->() const {
	return &owner->getSlot(identifier);
}

} // nodes
} // clover