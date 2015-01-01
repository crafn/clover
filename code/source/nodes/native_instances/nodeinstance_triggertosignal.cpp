#include "nodeinstance_triggertosignal.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* TriggerToSignalNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("trigger", SignalType::Trigger);
	n->addInputSlot("onValue", SignalType::Real, 1.0);
	n->addInputSlot("offValue", SignalType::Real, 0.0);

	n->addOutputSlot("signal", SignalType::Real);
	return n;
}

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
