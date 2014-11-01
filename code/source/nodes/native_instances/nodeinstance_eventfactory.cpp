#include "nodeinstance_eventfactory.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

void EventFactoryNodeInstance::create(){
	eventTypeInput= addInputSlot<SignalType::EventType>("eventType");
	triggerInput= addInputSlot<SignalType::Trigger>("trigger");
	
	eventOutput= addOutputSlot<SignalType::Event>("event");
	
	if (!eventTypeInput->get().empty())
		eventType= &resources::gCache->getResource<NodeEventType>(eventTypeInput->get());
	else
		eventType= &resources::gCache->getErrorResource<NodeEventType>();
	
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