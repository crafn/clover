#include "compositionnodelogic.hpp"
#include "nodefactory.hpp"
#include "nodetype.hpp"

namespace clover {
namespace nodes {

BaseCompositionNodeScriptLogic::BaseCompositionNodeScriptLogic()
		: batched(false)
		, batchPriority(0)
		, owner(nullptr)
		, updateRouteStart(false)
		, silent(false){
}

BaseCompositionNodeScriptLogic& BaseCompositionNodeScriptLogic::operator=(BaseCompositionNodeScriptLogic&& other){
	ensure(&other != this);
	
	owner= other.owner;
	
	// These are created/set when create() is run in script
	
	// templateGroups= std::move(other.templateGroups);
	// updateRouteStart= other.updateRouteStart;
	// resourceChangeListener= std::move(other.resourceChangeListener);

	const auto& this_slots= getSlotIdentifiers();
	const auto& other_slots= other.getSlotIdentifiers();
		
	auto preserved_slots= this_slots.duplicates(other_slots);
	auto added_slots= this_slots.removed(preserved_slots);
	auto removed_slots= other_slots.removed(preserved_slots);
	
	// Template slots are created again if slot template is not removed
	for (auto slot_it = removed_slots.begin(); slot_it != removed_slots.end();){
		
		const auto& slot_id= (*slot_it);
		bool saved= false;
		
		if (slot_id.groupName != ""){
			for (const auto& group_pair : other.templateGroups){
				if (group_pair.first == slot_id.groupName){
					preserved_slots.pushBack(slot_id);
					slot_it= removed_slots.erase(slot_it);
					saved= true;
					break;
				}
			}
		}
		
		if (!saved)
			++slot_it;
	}
	
	// Maintain connections
	for (const auto& m : preserved_slots){
		if (m.groupName != ""){
			ensure(other.getSlot(m).getIdentifier().groupName != "");
			addSlotMinimal(m); // Template slots aren't created in script like normal slots
		}
		
		getSlot(m)= std::move(other.getSlot(m));
	}
	

	// Inform UI
	for (const auto& m : removed_slots){
		if (!silent)
			sendOnSlotRemoveEvent(m);
	}
	
	for (const auto& m : added_slots){
		if (!silent)
			sendOnSlotAddEvent(m);
	}
	
	return *this;
}

BaseCompositionNodeScriptLogic::~BaseCompositionNodeScriptLogic(){
	clear();
}

CompositionNodeSlot& BaseCompositionNodeScriptLogic::addInputSlot(const SlotIdentifier& id, const boost::any& init_value){
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


CompositionNodeSlot& BaseCompositionNodeScriptLogic::addOutputSlot(const SlotIdentifier& id){
	ensure(!slots.count(id));

	auto& slot= addSlotMinimal(id);
	
	if (!silent)
		sendOnSlotAddEvent(id);
	
	//print(debug::Ch::Script, debug::Vb::Trivial, "Added output slot: %p", pair.first->second.get());
	
	return slot;
}

void BaseCompositionNodeScriptLogic::removeSlot(const SlotIdentifier& id){
	auto it= slots.find(id);
	ensure(it != slots.end());
	
	// Must detach before removing, otherwise detaching callbacks are fired when slot is destroyed
	it->second->detach();
	
	if (!silent)
		sendOnSlotRemoveEvent(id);
	
	slotArray.remove(it->second.get());
	slots.erase(it);
}

CompositionNodeSlot& BaseCompositionNodeScriptLogic::addSlot(const SlotIdentifier& id, boost::any init_value){
	if (id.input)
		return addInputSlot(id, init_value);
	return addOutputSlot(id);
}

CompositionNodeSlot& BaseCompositionNodeScriptLogic::addSlot(const SlotIdentifier& id){
	if (id.input)
		return addInputSlot(id);
	return addOutputSlot(id);
}

CompositionNodeSlot& BaseCompositionNodeScriptLogic::getSlot(const SlotIdentifier& id){
	auto it= slots.find(id);
	
	if (it == slots.end())
		throw resources::ResourceException("Couldn't find slot: %s from %s", id.getString().cStr(), owner->getType().getName().cStr());
	
	return *it->second;
}


util::DynArray<SlotIdentifier> BaseCompositionNodeScriptLogic::getSlotIdentifiers() const {
	util::DynArray<SlotIdentifier> ret;
	for (const auto& m : getSlots()){
		ret.pushBack(m->getIdentifier());
	}
	return (ret);
}


CompositionNodeSlotTemplateGroup& BaseCompositionNodeScriptLogic::addSlotTemplateGroup(const util::Str8& name, bool input){
	ensure(!templateGroups.count(name));
	ensure(owner);
	
	auto pair= templateGroups.insert(std::move(
		std::make_pair(name, CompositionNodeSlotTemplateGroupPtr(new CompositionNodeSlotTemplateGroup(*owner, name, input)))));
		
	return *pair.first->second;
}

const CompositionNodeSlotTemplateGroup& BaseCompositionNodeScriptLogic::getSlotTemplateGroup(const util::Str8& name) const {
	auto it= templateGroups.find(name);
	ensure_msg(it != templateGroups.end(), "Couldn't find template slot group: %s",
		name.cStr());
	return *it->second;
}

CompositionNodeSlotTemplateGroup& BaseCompositionNodeScriptLogic::getSlotTemplateGroup(const util::Str8& name){
	auto it= templateGroups.find(name);
	ensure_msg(it != templateGroups.end(), "Couldn't find template slot group: %s",
		name.cStr());
	return *it->second;
}

bool BaseCompositionNodeScriptLogic::hasSlotTemplateGroup(const util::Str8& name) const {
	auto it= templateGroups.find(name);
	if (it == templateGroups.end()) return false;
	return true;
}

void BaseCompositionNodeScriptLogic::setAsUpdateRouteStart(bool b){
	updateRouteStart= b;
}


void BaseCompositionNodeScriptLogic::setOwner(CompositionNodeLogic& owner_){
	owner= &owner_;
	
	ensure(slots.empty());
	ensure(resourceChangeListener.empty());
}

void BaseCompositionNodeScriptLogic::onDefaultValueChange(CompositionNodeSlot& slot){
	ensure(owner);
	owner->onDefaultValueChange(slot);
}

void BaseCompositionNodeScriptLogic::onRoutingChange(CompositionNodeSlot& slot){
	ensure(owner);
	owner->onRoutingChange(slot);
}

void BaseCompositionNodeScriptLogic::clear(){
	while (!slotArray.empty())
		removeSlot(*slotArray.back());
	resourceChangeListener.clear();
}


CompositionNodeSlot& BaseCompositionNodeScriptLogic::addSlotMinimal(const SlotIdentifier& id){
	auto pair= slots.insert(std::move(std::make_pair(
		id, CompositionNodeSlotPtr(new CompositionNodeSlot(id)))));
	pair.first->second->setOwner(owner);
	
	slotArray.pushBack(pair.first->second.get());
	
	return *pair.first->second;
}

void BaseCompositionNodeScriptLogic::sendOnSlotAddEvent(const SlotIdentifier& id_){
	ensure(!silent);
	
	SlotIdentifier id= id_;
	global::Event e(global::Event::OnNodeSlotAdd);
	e(global::Event::Node)= owner;
	e(global::Event::SlotIdentifier)= &id;
	e.send();
}

void BaseCompositionNodeScriptLogic::sendOnSlotRemoveEvent(const SlotIdentifier& id_){
	ensure(!silent);
	
	SlotIdentifier id= id_;
	global::Event e(global::Event::OnNodeSlotRemove);
	e(global::Event::Node)= owner;
	e(global::Event::SlotIdentifier)= &id;
	e.send();
}

void BaseCompositionNodeScriptLogic::addOnResourceChangeCallback(const resources::Resource& res){
	resourceChangeListener.listen(res, [&] () {
		ensure(owner);
		owner->onResourceChange(res);
	});
}

//
// CompositionNodeLogic
//

CompositionNodeLogic::CompositionNodeLogic(	const NodeType& type_,
											const script::ObjectType& obj_type,
											script::Context& context_)
	: type(&type_)
	, owner(nullptr)
	, context(context_)
	, objectType(obj_type)
	, impl(nullptr) {

	typeChangeListener.listen(*type, [&] () {
		recreate();
		util::OnChangeCb::trigger();
	});
	
	create();
}

void CompositionNodeLogic::create(){
	try {
		object= context.instantiateObject(objectType);
		runImplSetScript();
		runCreateScript();
	}
	catch (const resources::ResourceException& e){
		throw resources::ResourceException("CompositionNodeLogic::create failed for %s", getType().getName().cStr());
	}
}

void CompositionNodeLogic::recreate(){
	try {
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
	}
}

void CompositionNodeLogic::runImplSetScript(){
	auto get_impl= objectType.getMethod<BaseCompositionNodeScriptLogic& ()>("getImpl");
	impl= &context.execute(object, get_impl());
	ensure(impl);
	
	impl->setOwner(*this);
	
	onDefaultValueChangeFunc= objectType.getMethod<void (CompositionNodeSlot*)>("onDefaultValueChange");
	onResourceChangeFunc= objectType.getMethod<void (const resources::Resource*)>("onResourceChange");
}

void CompositionNodeLogic::runCreateScript(){
	auto create= objectType.getMethod<void ()>("create");
	context.execute(object, create());
}

CompositionNodeSlot& CompositionNodeLogic::getSlot(const SlotIdentifier& id) const {
	return impl->getSlot(id);
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

bool CompositionNodeLogic::hasSlot(const SlotIdentifier& id) const {
	return impl->hasSlot(id);
}

const NodeType& CompositionNodeLogic::getType() const {
	return *type;
}

util::DynArray<CompositionNodeSlot*> CompositionNodeLogic::getSlots() const {
	util::DynArray<CompositionNodeSlot*> ret;
	for (auto& m : impl->getSlots()){
		ret.pushBack(m);
	}
	
	return (ret);
}

util::DynArray<CompositionNodeSlot*> CompositionNodeLogic::getTemplateGroupSlots() const {
	util::DynArray<CompositionNodeSlot*> ret;
	
	for (auto& m : getSlots()){
		if (m->getTemplateGroup())
			ret.pushBack(m);
	}
	
	return (ret);
}

const CompositionNodeSlotTemplateGroup& CompositionNodeLogic::getSlotTemplateGroup(const util::Str8& name) const {
	return impl->getSlotTemplateGroup(name);
}

CompositionNodeSlotTemplateGroup& CompositionNodeLogic::getSlotTemplateGroup(const util::Str8& name){
	return impl->getSlotTemplateGroup(name);
}

bool CompositionNodeLogic::hasSlotTemplateGroup(const util::Str8& name) const {
	return impl->hasSlotTemplateGroup(name);
}

auto CompositionNodeLogic::getSlotTemplateGroups() const -> const util::HashMap<util::Str8, CompositionNodeSlotTemplateGroupPtr>& {
	return impl->getSlotTemplateGroups();
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
	impl->addSlot(slot.getIdentifier(), slot.getInitValue())
		.setTemplateGroup(&slot.getOwner());
}

void CompositionNodeLogic::removeSlot(const SlotTemplate& slot){
	impl->removeSlot(slot.getIdentifier());
}

void CompositionNodeLogic::onDefaultValueChange(CompositionNodeSlot& slot){
	
	// Thought this would have needed queuing, but works just like that
	bool found= false;
	for (auto& m : getSlots()){
		if (&slot == m){
			found= true;
			break;
		}
	}
	ensure(found);
	
	context.execute(object, onDefaultValueChangeFunc(&slot));
	
	util::OnChangeCb::trigger();
}

void CompositionNodeLogic::onRoutingChange(CompositionNodeSlot& slot){	
	util::OnChangeCb::trigger();
}

void CompositionNodeLogic::onResourceChange(const resources::Resource& res){
	ensure(object.isGood());
	ensure(onResourceChangeFunc.isGood());
	
	context.execute(object, onResourceChangeFunc(&res));
}

} // nodes
} // clover