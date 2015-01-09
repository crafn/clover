#include "nodeinstance_logicor.hpp"
#include "debug/print.hpp"
#include "resources/exception.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* LogicOrNodeInstance::compNode()
{
	CompositionNodeLogic* n= new CompositionNodeLogic{};
	n->addInputSlot("input1", SignalType::Boolean, false);
	n->addInputSlot("input2", SignalType::Boolean, false);
	n->addOutputSlot("output", SignalType::Boolean);
	return n;
}

void LogicOrNodeInstance::create()
{
	input1= addInputSlot<SignalType::Boolean>("input1");
	input2= addInputSlot<SignalType::Boolean>("input2");
	output= addOutputSlot<SignalType::Boolean>("output");

	auto recv= +[] (LogicOrNodeInstance* self) {
		self->setUpdateNeeded();
	};

	input1->setOnReceiveCallback(recv);
	input2->setOnReceiveCallback(recv);
}

void LogicOrNodeInstance::update()
{
	bool value= input1->get() || input2->get();
	output->send(value);
	setUpdateNeeded(false);

	print(debug::Ch::General, debug::Vb::Trivial, "LogicOr output: %i", value);
}

} // nodes
} // clover
