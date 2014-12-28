#include "nodeinstance_lerp_vec2.hpp"

namespace clover {
namespace nodes {

void Vec2LerpNodeInstance::create()
{
	in1= addInputSlot<SignalType::Vec2>("input1");
	in2= addInputSlot<SignalType::Vec2>("input2");
	factorIn= addInputSlot<SignalType::Real>("factor");

	resultOut= addOutputSlot<SignalType::Vec2>("result");

	auto recv= [&] ()
	{ setUpdateNeeded(true); };
	in1->setOnReceiveCallback(recv);
	in2->setOnReceiveCallback(recv);
	factorIn->setOnReceiveCallback(recv);

	setUpdateNeeded(false);
}

void Vec2LerpNodeInstance::update()
{
	real64 factor= factorIn->get();
	resultOut->send(in1->get()*(1.0 - factor) + in2->get()*factor);
	setUpdateNeeded(false);
}

} // nodes
} // clover
