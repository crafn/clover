#include "global/env.hpp"
#include "nodeinstance_eventhandler.hpp"
#include "nodes/nodeeventtype.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

class EventHandlerCompositionNodeLogic : public CompositionNodeLogic {
public:
	EventHandlerCompositionNodeLogic()
	{
		eventTypeSlot= &addInputSlot("eventType", SignalType::EventType);
		addInputSlot("events", SignalType::EventArray);
		addOutputSlot("forward", SignalType::Event);
	}

	bool hasEventType()
	{ return eventTypeSlot->getDefaultValue<SignalType::EventType>() != ""; }

	void onDefaultValueChange(CompositionNodeSlot& slot) override
	{
		if (&slot == eventTypeSlot && hasEventType()){
			clearResourceChangeListeners();
			addResourceChangeListener<NodeEventType>(eventTypeSlot->getDefaultValue<SignalType::EventType>());
			recreateArgSlots();
		}
		CompositionNodeLogic::onDefaultValueChange(slot);
	}

	void onResourceChange(const resources::Resource& res){
		recreateArgSlots();	
	}

	void recreateArgSlots(){
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
		for (SizeType i= 0; i < args.size(); ++i){
			SlotIdentifier id{args[i].name, "arg", args[i].signalType, false}; // is_input == false

			if (hasSlot(id)){
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

CompositionNodeLogic* EventHandlerNodeInstance::compNode()
{ return new EventHandlerCompositionNodeLogic{}; }

void EventHandlerNodeInstance::create(){
	eventTypeInput= addInputSlot<SignalType::EventType>("eventType");
	eventsInput= addInputSlot<SignalType::EventArray>("events");

	forwardOutput= addOutputSlot<SignalType::Event>("forward");
	
	if (!eventTypeInput->get().empty())
		eventType= &global::g_env->resCache->getResource<NodeEventType>(eventTypeInput->get());
	else
		eventType= &global::g_env->resCache->getErrorResource<NodeEventType>();
	
	for (auto& m : eventType->getArguments()){
		argumentOutputs[m.name]= addOutputSlot(m.name, "arg", m.signalType);
	}
	
	setUpdateNeeded(false);
	
	eventsInput->setOnReceiveCallback([&] () {
		if (eventsInput->get().empty())
			return;
		
		const auto& name= eventType->getName();
		for (const auto& event : eventsInput->get()){
			if (event.getName() != name) // Accept only events of corresponding type
				continue;
			
			events.pushBack(event);
		}
		
		if (!events.empty())
			setUpdateNeeded(); // Send some events in the next update
	});
	
}

void EventHandlerNodeInstance::update(){
	ensure(!events.empty());
	
	// Handle one event per frame
	const auto& event= events.front();
	
	//print(debug::Ch::WE, debug::Vb::Trivial, "NodeEvent %s handled, arg count: %zu",
	//	event.getName().cStr(), event.getArgCount());
	
	for (auto& m : argumentOutputs){
		m.second->send(event.get(m.first));
	}

	forwardOutput->send(event);
	
	events.popFront();
	
	if (events.empty())
		setUpdateNeeded(false);
}

} // nodes
} // clover
