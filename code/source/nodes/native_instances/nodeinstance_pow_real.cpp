#include "nodeinstance_pow_real.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* RealPowNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("base", SignalType::Real, 0.0);
	n->addInputSlot("exponent", SignalType::Real, 0.0);
	n->addOutputSlot("output", SignalType::Real);
	return n;
}

void RealPowNodeInstance::create()
{
	baseInput= addInputSlot<SignalType::Real>("base");
	exponentInput= addInputSlot<SignalType::Real>("exponent");

	realOut= addOutputSlot<SignalType::Real>("output");

	auto recv= [&] ()
	{ realOut->send(pow(baseInput->get(), exponentInput->get())); };
	baseInput->setOnReceiveCallback(recv);
	exponentInput->setOnReceiveCallback(recv);

	setUpdateNeeded(false);
}

} // nodes
} // clover
