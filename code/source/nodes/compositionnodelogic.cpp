#include "compositionnodelogic.hpp"
#include "global/event.hpp"
#include "nodefactory.hpp"
#include "nodetype.hpp"

namespace clover {
namespace nodes {

CompositionNodeSlot& CompositionNodeLogic::addInputSlot(const SlotIdentifier& id, const boost::any& init_value){
	ensure(!slots.count(id));
	
	auto& slot= addSlotMinimal(id);
	
	try {
		slot.setInitValue(init_value);
	}
	catch (global::Exception&){
		print(debug::Ch::Nodes, debug::Vb::Critical, "Setting init value failed for slot: %s", id.getString().cStr());
		slot.setInitValue(RuntimeSignalTypeTraits::defaultInitValue(id.signalType));
	}
	
	if (!silent)
		sendOnSlotAddEvent(id);
	
	return slot;
}


CompositionNodeSlot& CompositionNodeLogic::addOutputSlot(const SlotIdentifier& id){
	ensure(!slots.count(id));

	auto& slot= addSlotMinimal(id);
	
	if (!silent)
		sendOnSlotAddEvent(id);
	
	//print(debug::Ch::Script, debug::Vb::Trivial, "Added output slot: %p", pair.first->second.get());
	
	return slot;
}

void CompositionNodeLogic::removeSlot(const SlotIdentifier& id){
	auto it= slots.find(id);
	ensure(it != slots.end());
	
	// Must detach before removing, otherwise detaching callbacks are fired when slot is destroyed
	it->second->detach();
	
	if (!silent)
		sendOnSlotRemoveEvent(id);
	
	slotArray.remove(it->second.get());
	slots.erase(it);
}

CompositionNodeSlot& CompositionNodeLogic::addSlot(const SlotIdentifier& id, boost::any init_value){
	if (id.input)
		return addInputSlot(id, init_value);
	return addOutputSlot(id);
}

CompositionNodeSlot& CompositionNodeLogic::addSlot(const SlotIdentifier& id){
	if (id.input)
		return addInputSlot(id);
	return addOutputSlot(id);
}

CompositionNodeSlot& CompositionNodeLogic::getSlot(const SlotIdentifier& id) const {
	auto it= slots.find(id);
	
	if (it == slots.end())
		throw resources::ResourceException("Couldn't find slot: %s from %s", id.getString().cStr(), getType().getName().cStr());
	
	return *it->second;
}


util::DynArray<SlotIdentifier> CompositionNodeLogic::getSlotIdentifiers() const {
	util::DynArray<SlotIdentifier> ret;
	for (const auto& m : getSlots()){
		ret.pushBack(m->getIdentifier());
	}
	return (ret);
}


CompositionNodeSlotTemplateGroup& CompositionNodeLogic::addSlotTemplateGroup(const util::Str8& name, bool input){
	ensure(!templateGroups.count(name));
	
	auto pair= templateGroups.insert(std::move(
		std::make_pair(name, CompositionNodeSlotTemplateGroupPtr(new CompositionNodeSlotTemplateGroup(*this, name, input)))));
		
	return *pair.first->second;
}

const CompositionNodeSlotTemplateGroup& CompositionNodeLogic::getSlotTemplateGroup(const util::Str8& name) const {
	auto it= templateGroups.find(name);
	ensure_msg(it != templateGroups.end(), "Couldn't find template slot group: %s",
		name.cStr());
	return *it->second;
}

CompositionNodeSlotTemplateGroup& CompositionNodeLogic::getSlotTemplateGroup(const util::Str8& name){
	auto it= templateGroups.find(name);
	ensure_msg(it != templateGroups.end(), "Couldn't find template slot group: %s",
		name.cStr());
	return *it->second;
}

bool CompositionNodeLogic::hasSlotTemplateGroup(const util::Str8& name) const {
	auto it= templateGroups.find(name);
	if (it == templateGroups.end()) return false;
	return true;
}

void CompositionNodeLogic::setAsUpdateRouteStart(bool b){
	updateRouteStart= b;
}

void CompositionNodeLogic::clear(){
	while (!slotArray.empty())
		removeSlot(*slotArray.back());
	resourceChangeListener.clear();
}


CompositionNodeSlot& CompositionNodeLogic::addSlotMinimal(const SlotIdentifier& id){
	auto pair= slots.insert(std::move(std::make_pair(
		id, CompositionNodeSlotPtr(new CompositionNodeSlot(id)))));
	pair.first->second->setOwner(this);
	slotArray.pushBack(pair.first->second.get());
	
	return *pair.first->second;
}

void CompositionNodeLogic::sendOnSlotAddEvent(const SlotIdentifier& id_){
	ensure(!silent);
	
	SlotIdentifier id= id_;
	global::Event e(global::Event::OnNodeSlotAdd);
	e(global::Event::Node)= this;
	e(global::Event::SlotIdentifier)= &id;
	e.send();
}

void CompositionNodeLogic::sendOnSlotRemoveEvent(const SlotIdentifier& id_){
	ensure(!silent);
	
	SlotIdentifier id= id_;
	global::Event e(global::Event::OnNodeSlotRemove);
	e(global::Event::Node)= this;
	e(global::Event::SlotIdentifier)= &id;
	e.send();
}

void CompositionNodeLogic::addOnResourceChangeCallback(const resources::Resource& res){
	resourceChangeListener.listen(res, [&] () {
		onResourceChange(res);
	});
}

//
// CompositionNodeLogic
//

CompositionNodeLogic::CompositionNodeLogic()
	: batched(false)
	, batchPriority(0)
	, updateRouteStart(false)
	, silent(false)
	, type(nullptr)
	, owner(nullptr) {

}

void CompositionNodeLogic::setType(const NodeType& type_)
{
	type= &type_;
	typeChangeListener.listen(*type, [&] () {
		recreate();
		util::OnChangeCb::trigger();
	});
}

void CompositionNodeLogic::recreate(){
	release_ensure_msg(false, "Not implemented");
	/*try {
		// Take changes in script into account without breaking any connections etc.
		
		script::Object old_object= std::move(object);
		auto old_impl= impl;
		old_impl->setSilent(true);
		
		object= context.instantiateObject(objectType);
		runImplSetScript();
		impl->setSilent(true);
		runCreateScript();

		// Trigger default value changes, because move won't trigger
		for (auto& m : getSlots()){
			if (m->isDefaultValueSet())
				context.execute(object, onDefaultValueChangeFunc(m));
		}
		
		impl->setSilent(false);
		
		*impl= std::move(*old_impl); // Move slots to new from old
	}
	catch (const resources::ResourceException& e){
		throw resources::ResourceException("CompositionNodeLogic::recreate failed for %s", getType().getName().cStr());
	}*/
}

CompositionNodeSlot& CompositionNodeLogic::getInputSlot(const SlotIdentifier& id) const {
	auto& slot= getSlot(id);
	release_ensure(slot.getIdentifier().input);
	return slot;
}

CompositionNodeSlot& CompositionNodeLogic::getOutputSlot(const SlotIdentifier& id) const {
	auto& slot= getSlot(id);
	release_ensure(!slot.getIdentifier().input);
	return slot;
}

const NodeType& CompositionNodeLogic::getType() const {
	return *type;
}

util::DynArray<CompositionNodeSlot*> CompositionNodeLogic::getTemplateGroupSlots() const {
	util::DynArray<CompositionNodeSlot*> ret;
	
	for (auto& m : getSlots()){
		if (m->getTemplateGroup())
			ret.pushBack(m);
	}
	
	return (ret);
}

util::DynArray<CompositionNodeSlotTemplateGroup*> CompositionNodeLogic::getInputSlotTemplateGroups() const {
	util::DynArray<CompositionNodeSlotTemplateGroup*> ret;
	for (auto& m : getSlotTemplateGroups()){
		if (m.second->isInput())
			ret.pushBack(m.second.get());
	}
	return (ret);
}

util::DynArray<CompositionNodeSlotTemplateGroup*> CompositionNodeLogic::getOutputSlotTemplateGroups() const {
	util::DynArray<CompositionNodeSlotTemplateGroup*> ret;
	for (auto& m : getSlotTemplateGroups()){
		if (!m.second->isInput())
			ret.pushBack(m.second.get());
	}
	return (ret);
}

util::DynArray<CompositionNodeSlotTemplateGroup*> CompositionNodeLogic::getSlotTemplateGroups(bool input) const {
	if (input)
		return getInputSlotTemplateGroups();
	else
		return getOutputSlotTemplateGroups();
}

void CompositionNodeLogic::addSlot(const SlotTemplate& slot){
	addSlot(slot.getIdentifier(), slot.getInitValue())
		.setTemplateGroup(&slot.getOwner());
}

void CompositionNodeLogic::removeSlot(const SlotTemplate& slot){
	removeSlot(slot.getIdentifier());
}

void CompositionNodeLogic::onDefaultValueChange(CompositionNodeSlot& slot){
	util::OnChangeCb::trigger();
}

void CompositionNodeLogic::onRoutingChange(CompositionNodeSlot& slot){	
	util::OnChangeCb::trigger();
}

} // nodes
} // clover
