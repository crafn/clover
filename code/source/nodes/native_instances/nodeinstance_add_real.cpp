#include "nodeinstance_add_real.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* RealAddNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("input1", SignalType::Real, 0.0);
	n->addInputSlot("input2", SignalType::Real, 0.0);
	n->addOutputSlot("result", SignalType::Real);
	return n;
}

void RealAddNodeInstance::create()
{
	input1= addInputSlot<SignalType::Real>("input1");
	input2= addInputSlot<SignalType::Real>("input2");
	realOut= addOutputSlot<SignalType::Real>("result");

	auto recv= [&]
	{ realOut->send(input1->get() + input2->get()); };

	input1->setOnReceiveCallback(recv);
	input2->setOnReceiveCallback(recv);

	setUpdateNeeded(false);
}

} // nodes
} // clover
