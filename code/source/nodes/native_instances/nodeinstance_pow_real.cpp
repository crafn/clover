#include "nodeinstance_pow_real.hpp"

namespace clover {
namespace nodes {

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
