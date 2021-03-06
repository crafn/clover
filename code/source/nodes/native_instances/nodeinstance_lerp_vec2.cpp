#include "nodeinstance_lerp_vec2.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* Vec2LerpNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("input1", SignalType::Vec2);
	n->addInputSlot("input2", SignalType::Vec2);
	n->addInputSlot("factor", SignalType::Real);

	n->addOutputSlot("result", SignalType::Vec2);
	return n;
}

void Vec2LerpNodeInstance::create()
{
	in1= addInputSlot<SignalType::Vec2>("input1");
	in2= addInputSlot<SignalType::Vec2>("input2");
	factorIn= addInputSlot<SignalType::Real>("factor");

	resultOut= addOutputSlot<SignalType::Vec2>("result");

	auto recv= +[] (Vec2LerpNodeInstance* self)
	{ self->setUpdateNeeded(true); };
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
