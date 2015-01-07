#include "compositionnodeslot.hpp"
#include "compositionnodelogic.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace nodes {

bool CompositionNodeSlot::AttachedSlotInfo::operator==(const AttachedSlotInfo& other){
	if (slot == other.slot && mySub == other.mySub && slotSub == other.slotSub)
		return true;
	return false;
}

CompositionNodeSlot::CompositionNodeSlot(const SlotIdentifier& id):
	owner(nullptr),
	identifier(id),
	templateGroup(nullptr){
}

CompositionNodeSlot& CompositionNodeSlot::operator=(CompositionNodeSlot&& other){
	owner= other.owner;
	identifier= other.identifier;
	templateGroup= other.templateGroup;
	attachedSlotInfos= other.attachedSlotInfos;
	initValue= other.initValue;
	defaultValue= other.defaultValue;
	other.attachedSlotInfos.clear();
	
	return *this;
}

CompositionNodeSlot::~CompositionNodeSlot(){
	detach();
}

void CompositionNodeSlot::setOwner(CompositionNodeLogic* owner_){
	owner= owner_;
	ensure(owner);
}

const CompositionNodeLogic& CompositionNodeSlot::getOwner() const {
	ensure(owner);
	return *owner;
}

CompositionNodeLogic& CompositionNodeSlot::getOwner(){
	ensure(owner);
	return *owner;
}


void CompositionNodeSlot::attach(CompositionNodeSlot& other, SubSignalType from, SubSignalType to){
	attachImpl(from, to, other);
	other.attachImpl(to, from, *this);
}

void CompositionNodeSlot::detach(CompositionNodeSlot& other, SubSignalType from, SubSignalType to){
	
	auto it= attachedSlotInfos.find(AttachedSlotInfo{CompositionNodeSlotHandle(other), from, to});
	if (it == attachedSlotInfos.end())
		return;
	
	attachedSlotInfos.erase(it);
	
	other.detach(*this, to, from);
	
	getOwner().onRoutingChange(*this);
}

void CompositionNodeSlot::detach(){
	while (!attachedSlotInfos.empty()){
		detach(*NONULL(attachedSlotInfos.back().slot.get()), attachedSlotInfos.back().mySub, attachedSlotInfos.back().slotSub);
	}
}

bool CompositionNodeSlot::isAttached() const {
	return !attachedSlotInfos.empty();
}

bool CompositionNodeSlot::isAttached(SubSignalType to) const {
	
	for (auto& m : attachedSlotInfos){
		if (m.mySub == to) return true;
	}
	return false;
}

auto CompositionNodeSlot::getAttachedSlotInfos() const -> const util::DynArray<AttachedSlotInfo>& {
	return attachedSlotInfos;
}

util::DynArray<CompositionNodeSlot*> CompositionNodeSlot::getAttachedSlots() const {
	util::DynArray<CompositionNodeSlot*> ret;
	
	for (auto& m : attachedSlotInfos){
		ret.pushBack(m.slot.get());
	}
	
	return (ret);
}

bool CompositionNodeSlot::isCompatible(SubSignalType from_type, SubSignalType to_type, CompositionNodeSlot& other) const {
	if (isInput() == other.isInput()) return false;
	
	SignalType from= identifier.signalType;
	SignalType to= other.identifier.signalType;
	
	if (from_type != SubSignalType::None)
		from= RuntimeSubSignalTypeTraits::signalType(from_type);
		
	if (to_type != SubSignalType::None)
		to= RuntimeSubSignalTypeTraits::signalType(to_type);

	// SubSignalling
	ensure(identifier.signalType == from || hasSubSignalType(from_type));
	ensure(other.identifier.signalType == to || other.hasSubSignalType(to_type));
	
	return from == to;
	
}

bool CompositionNodeSlot::hasSubSignalType(SubSignalType type) const {
	for (auto& m : RuntimeSignalTypeTraits::subTypes(identifier.signalType)){
		if (type == m) return true;
	}
	return false;
}

void CompositionNodeSlot::setInitValue(const util::Any& v){
	ensure(isInput());
	ensure(!v.empty());
	RuntimeSignalTypeTraits::checkValue(identifier.signalType, v);
	initValue= v;
	ensure(!initValue.empty());
}

void CompositionNodeSlot::setDefaultValue(const util::Any& v){
	ensure(isInput());
	defaultValue= v;
	RuntimeSignalTypeTraits::checkValue(identifier.signalType, defaultValue);
	
	getOwner().onDefaultValueChange(*this);
}

void CompositionNodeSlot::unsetDefaultValue(){
	defaultValue= util::Any();
	getOwner().onDefaultValueChange(*this);
}

const util::Any& CompositionNodeSlot::getDefaultValue() const {
	ensure(isInput());
	ensure(!initValue.empty());
	
	if (defaultValue.empty()){
		return initValue;
	}
	
	return defaultValue;
}

util::Str8 CompositionNodeSlot::getTemplateGroupName() const {
	return templateGroup ? templateGroup->getName() : util::Str8("");
}

void CompositionNodeSlot::attachImpl(SubSignalType from, SubSignalType to, CompositionNodeSlot& other){
	ensure(isCompatible(from, to, other));
	
	auto it= attachedSlotInfos.find(AttachedSlotInfo{CompositionNodeSlotHandle(other), from, to});
	
	ensure_msg(it == attachedSlotInfos.end(), "Routing multiple outputs to a single (sub)slot is not supported");
	
	attachedSlotInfos.pushBack(AttachedSlotInfo{CompositionNodeSlotHandle(other), from, to});
	ensure(attachedSlotInfos.back().slot.get());
	
	getOwner().onRoutingChange(*this);
	
}

} // nodes
} // clover
