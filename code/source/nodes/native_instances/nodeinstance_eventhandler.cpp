#include "nodeinstance_eventhandler.hpp"
#include "resources/cache.hpp"
#include "nodes/nodeeventtype.hpp"

namespace clover {
namespace nodes {

void EventHandlerNodeInstance::create(){
	eventTypeInput= addInputSlot<SignalType::EventType>("eventType");
	eventsInput= addInputSlot<SignalType::EventArray>("events");

	forwardOutput= addOutputSlot<SignalType::Event>("forward");
	
	if (!eventTypeInput->get().empty())
		eventType= &resources::gCache->getResource<NodeEventType>(eventTypeInput->get());
	else
		eventType= &resources::gCache->getErrorResource<NodeEventType>();
	
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