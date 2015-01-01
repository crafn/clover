#include "nodeinstance_delayedtrigger.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* DelayedTriggerNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("trigger", SignalType::Trigger);
	n->addInputSlot("delay", SignalType::Real, 1.0);
	n->addOutputSlot("trigger", SignalType::Trigger);
	return n;
}

void DelayedTriggerNodeInstance::create()
{
	triggerIn= addInputSlot<SignalType::Trigger>("trigger");
	timeIn= addInputSlot<SignalType::Real>("delay");
	triggerOut= addOutputSlot<SignalType::Trigger>("trigger");
	
	triggerIn->setOnReceiveCallback([&] ()
	{
		timer= timeIn->get();
		setUpdateNeeded(true);
	});

	timer= 0.1;
	setUpdateNeeded(false); 
}

void DelayedTriggerNodeInstance::update()
{
	timer -= util::gGameClock->getDeltaTime();

	if (timer <= 0.0){
		timer= 0.0;
		setUpdateNeeded(false);
		triggerOut->send();
	}
}

} // nodes
} // clover
