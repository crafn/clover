#include "compositionnodeslottemplategrouphandle.hpp"
#include "compositionnodelogic.hpp"
#include "compositionnodeslottemplategroup.hpp"

namespace clover {
namespace nodes {

CompositionNodeSlotTemplateGroupHandle::CompositionNodeSlotTemplateGroupHandle(CompositionNodeSlotTemplateGroup* group)
	: owner(nullptr){
	if (group){
		owner= &group->getOwner();
		name= group->getName();
	}
}

CompositionNodeSlotTemplateGroupHandle::operator bool() const {
	if (!owner) return false;
	return owner->hasSlotTemplateGroup(name);
}

CompositionNodeSlotTemplateGroup* CompositionNodeSlotTemplateGroupHandle::get() const {
	if (*this)
		return &owner->getSlotTemplateGroup(name);
	return nullptr;
}

CompositionNodeSlotTemplateGroup* CompositionNodeSlotTemplateGroupHandle::operator->() const {
	return &owner->getSlotTemplateGroup(name);
}

} // nodes
} // clover