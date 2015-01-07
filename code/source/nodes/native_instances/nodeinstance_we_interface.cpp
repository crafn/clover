#include "nodeinstance_we_interface.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* WeInterfaceNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->setAsUpdateRouteStart(true);

	n->addInputSlot("active", SignalType::Boolean, false);
	n->addInputSlot("remove", SignalType::Trigger);

	n->addOutputSlot("active", SignalType::Boolean);
	n->addOutputSlot("events", SignalType::EventArray);
	n->addOutputSlot("we", SignalType::WeHandle);
	
	CompositionNodeSlotTemplateGroup& attribs_group= n->addOutputSlotTemplateGroup("attribs");
	attribs_group.setAsVariant(true);
	attribs_group.setMirroring(true);
	attribs_group.addSlotTemplate("transform", SignalType::RtTransform2);
	attribs_group.addSlotTemplate("gridChange", SignalType::Trigger);
	return n;
}

WeInterfaceNodeInstance::~WeInterfaceNodeInstance(){
	if (onDestroy)
		onDestroy();
}

void WeInterfaceNodeInstance::setEntity(game::WorldEntity& we){
	ensure(!entity);
	entity= &we;
}

void WeInterfaceNodeInstance::create(){
	activeInput= addInputSlot<SignalType::Boolean>("active");
	removeInput= addInputSlot<SignalType::Trigger>("remove");
	
	activeOutput= addOutputSlot<SignalType::Boolean>("active");
	eventsOutput= addOutputSlot<SignalType::EventArray>("events");
	weOutput= addOutputSlot<SignalType::WeHandle>("we");
	
	removeInput->setOnReceiveCallback([this] (){
		removed= true;
	});
	
	// Create attribute slots
	for (const auto& comp_slot : compositionNodeLogic->getTemplateGroupSlots()){
		if (comp_slot->isInput()){
			BaseInputSlot* slot=	addInputSlot(	comp_slot->getName(),
													comp_slot->getTemplateGroupName(),
													comp_slot->getSignalType());
			attributeInputs[hash32(comp_slot->getName())]= slot;
			
			if (comp_slot->getName() == "transform")
				transformInput= slot;
		}
		else {
			BaseOutputSlot* slot= addOutputSlot(	comp_slot->getName(),
													comp_slot->getTemplateGroupName(),
													comp_slot->getSignalType());
			attributeOutputs[hash32(comp_slot->getName())]= slot;
			
			if (comp_slot->getName() == "transform")
				transformOutput= slot;
		}
	}
	
	setUpdateNeeded();
	firstUpdate= true;
}

void WeInterfaceNodeInstance::update(){
	//print(debug::Ch::General, debug::Vb::Trivial, "WeInterface transform: %f, %f", transformInput->get().translation.x, transformInput->get().translation.y);
	
	if (firstUpdate){
		weOutput->send(game::WeHandle{entity});

		sendAttributes();
		firstUpdate= false;
	}
	
	if (!events.empty()){
		eventsOutput->send(events);
		events.clear();
	}
	
	setUpdateNeeded(false);
}

void WeInterfaceNodeInstance::setActive(bool b){
	ensure(activeInput);
	ensure(activeOutput);
	activeInput->set(b);
	activeOutput->send(activeInput->get());
}	

void WeInterfaceNodeInstance::setPosition(util::Vec2d t){
	if (transformInput){
		if (transformInput->getType() == SubSignalTypeTraits<SubSignalType::Translation2>::signalType){
			transformInput->set(t);
		}
		else {
			transformInput->subSet(SubSignalType::Translation2, t, SubSignalType::Translation2);
		}
	}
	
	if (transformOutput && transformInput)
		transformOutput->send(transformInput->getAny());
}

util::Vec2d WeInterfaceNodeInstance::getPosition(){
	if (!transformInput){
		return util::Vec2d();
	}
	else {
		return transformInput->subGet<SubSignalType::Translation2>();
	}
}


void WeInterfaceNodeInstance::receiveEvent(const NodeEvent& e){
	events.pushBack(e);
	setUpdateNeeded();
}

bool WeInterfaceNodeInstance::hasAttribute(const util::Str8& name) const {
	return attributeInputs.find(hash32(name)) != attributeInputs.end();
}

void WeInterfaceNodeInstance::setAttribute(const util::Str8& name, const util::Any& value){
	setAttribute(hash32(name), value, name);
}

void WeInterfaceNodeInstance::setAttributes(const AttributeInfos& attribs){
	for (const auto& a : attribs){
		setAttribute(a.nameHash, a.value);
	}
}

WeInterfaceNodeInstance::AttributeInfos WeInterfaceNodeInstance::getAttributeInfos() const {
	AttributeInfos attribs;
	for (const auto& pair : attributeInputs){
		const auto& slot= *pair.second;
		attribs.pushBack(AttributeInfo{pair.first, slot.getType(), slot.getAny()});
	}
	return attribs;
}

void WeInterfaceNodeInstance::setAttribute(uint32 name_hash, const util::Any& value, const util::Str8& disp_name){
	auto i_it= attributeInputs.find(name_hash);
	
	if (i_it == attributeInputs.end()){
		print(debug::Ch::WE, debug::Vb::Moderate, "WeInterfaceNodeInstance::setAttribute(..): attribute (input slot) not found: %s, %u", disp_name.cStr(), name_hash);
		return;
	}
	
	try {
		i_it->second->set(value);
	}
	catch (const global::Exception& e){
		print(debug::Ch::WE, debug::Vb::Moderate, "WeInterfaceNodeInstance::setAttribute(..): setting value failed for: %s, %u", disp_name.cStr(), name_hash);
		/// @todo Check types and try to convert value, e.g. util::RtTransform is easy to convert util::SrtTransform
	}

	if (!firstUpdate){
		auto o_it= attributeOutputs.find(name_hash);
		ensure(o_it != attributeOutputs.end());
		o_it->second->send(value);
	}
}

void WeInterfaceNodeInstance::sendAttributes(){
	for (auto i_pair : attributeInputs){
		auto o_it= attributeOutputs.find(i_pair.first);
		
		if (o_it == attributeOutputs.end())
			throw global::Exception("WeInterfaceNodeInstance::setAttribute(..): attribute (output) not found");
			
		o_it->second->send(i_pair.second->getAny());
	}
}

} // nodes
} // clover
