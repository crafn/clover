#include "nodeinstance_logicor.hpp"
#include "debug/debugprint.hpp"
#include "resources/exception.hpp"

namespace clover {
namespace nodes {

void LogicOrNodeInstance::create(){
	input1= addInputSlot<SignalType::Boolean>("input1");
	input2= addInputSlot<SignalType::Boolean>("input2");
	output= addOutputSlot<SignalType::Boolean>("output");
	
	auto recv = [&] () {
		setUpdateNeeded();
	};
		
	input1->setOnReceiveCallback(recv);
	input2->setOnReceiveCallback(recv);
}

void LogicOrNodeInstance::update(){
	bool value= input1->get() || input2->get();
	output->send(value);
	setUpdateNeeded(false);
	
	print(debug::Ch::General, debug::Vb::Trivial, "LogicOr output: %i", value);
}

} // nodes
} // clover