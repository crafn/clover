#include "nodeinstance_triggertosignal.hpp"

namespace clover {
namespace nodes {

void TriggerToSignalNodeInstance::create()
{
	triggerIn= addInputSlot<SignalType::Trigger>("trigger");
	onValueIn= addInputSlot<SignalType::Real>("onValue");
	offValueIn= addInputSlot<SignalType::Real>("offValue");

	signalOut= addOutputSlot<SignalType::Real>("signal");

	triggerIn->setOnReceiveCallback([&] ()
	{ triggerReceived= true; });
	triggerReceived= false;

	setUpdateNeeded(true);
}

void TriggerToSignalNodeInstance::update()
{
	if (triggerReceived)
		signalOut->send(onValueIn->get());
	else
		signalOut->send(offValueIn->get());

	triggerReceived= false;
}

} // nodes
} // clover
