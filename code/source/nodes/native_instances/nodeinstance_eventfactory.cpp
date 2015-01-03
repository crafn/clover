#include "global/env.hpp"
#include "nodeinstance_eventfactory.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

class EventFactoryCompositionNodeLogic : public CompositionNodeLogic {
public:
	EventFactoryCompositionNodeLogic()
	{
		eventTypeSlot= &addInputSlot("eventType", SignalType::EventType, util::Str8(""));
		addInputSlot("trigger", SignalType::Trigger);
		addOutputSlot("event", SignalType::Event);
	}

	bool hasEventType()
	{ return eventTypeSlot->getDefaultValue<SignalType::EventType>() != ""; }

	void onDefaultValueChange(CompositionNodeSlot& slot) override
	{
		if (&slot == eventTypeSlot && hasEventType()){
			recreateArgSlots();
			clearResourceChangeListeners();
			addResourceChangeListener<NodeEventType>(eventTypeSlot->getDefaultValue<SignalType::EventType>());
		}
		CompositionNodeLogic::onDefaultValueChange(slot);
	}

	void onResourceChange(const resources::Resource& res) override
	{ recreateArgSlots(); }

	void recreateArgSlots()
	{
		if (!hasEventType())
			return;

		util::DynArray<SignalArgument> args=
			global::g_env->resCache->getResource<NodeEventType>(
					eventTypeSlot->getDefaultValue<SignalType::EventType>()
			).getArguments();
			

		// Remove slots of old EventType
		for (SizeType i= 0; i < argSlots.size(); ++i){
			if (args.count(SignalArgument(argSlots[i]->getName(), argSlots[i]->getSignalType())) == 0)
				removeSlot(*argSlots[i]);
		}

		argSlots.clear();

		// Add new
		for (SizeType i= 0; i < args.size(); ++i) {
			SlotIdentifier id{args[i].name, "arg", args[i].signalType, true}; // is_input == true

			if (hasSlot(id)) {
				argSlots.pushBack(&getSlot(id));
			} else {
				argSlots.pushBack(&addSlot(id));
			}
		}
	}

private:
	CompositionNodeSlot* eventTypeSlot;
	util::DynArray<CompositionNodeSlot*> argSlots;
};

CompositionNodeLogic* EventFactoryNodeInstance::compNode()
{ return new EventFactoryCompositionNodeLogic{}; }

void EventFactoryNodeInstance::create(){
	eventTypeInput= addInputSlot<SignalType::EventType>("eventType");
	triggerInput= addInputSlot<SignalType::Trigger>("trigger");
	
	eventOutput= addOutputSlot<SignalType::Event>("event");
	
	if (!eventTypeInput->get().empty())
		eventType= &global::g_env->resCache->getResource<NodeEventType>(eventTypeInput->get());
	else
		eventType= &global::g_env->resCache->getErrorResource<NodeEventType>();
	
	for (auto& m : eventType->getArguments()){
		argumentInputs[m.name]= addInputSlot(m.name, "arg", m.signalType);
	}
	
	setUpdateNeeded(false);
	
	triggerInput->setOnReceiveCallback([&] () {
		ensure(eventType);
		NodeEvent event(*eventType);
		
		//print(debug::Ch::WE, debug::Vb::Trivial, "NodeEvent %s signal sent",
		//	eventType->getName().cStr());
		
		for (auto& m : argumentInputs){
			event.set(m.first, m.second->getAny());
		}
		
		eventOutput->send(event);
	});
}

void EventFactoryNodeInstance::update(){
}

} // nodes
} // clover
