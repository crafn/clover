#include "nodeinstance_multiply_real.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* RealMultiplyNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("input1", SignalType::Real, 0.0);
	n->addInputSlot("input2", SignalType::Real, 0.0);
	n->addOutputSlot("result", SignalType::Real);
	return n;
}

void RealMultiplyNodeInstance::create()
{
	input1= addInputSlot<SignalType::Real>("input1");
	input2= addInputSlot<SignalType::Real>("input2");
	realOut= addOutputSlot<SignalType::Real>("result");

	auto recv= +[] (RealMultiplyNodeInstance* self)
	{ self->realOut->send(self->input1->get()*self->input2->get()); };

	input1->setOnReceiveCallback(recv);
	input2->setOnReceiveCallback(recv);

	setUpdateNeeded(false);
}

} // nodes
} // clover
