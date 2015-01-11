#include "baseinputslot.hpp"
#include "debug/print.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace nodes {
	
BaseInputSlot::BaseInputSlot(SignalType t)
	: BaseSlot(t)
	, valueReceived(false)
	, onReceiveCallback(nullptr)
{ }

BaseInputSlot::~BaseInputSlot(){
}

void BaseInputSlot::update(NodeInstance& inst){
	if (valueReceived && onReceiveCallback) {
		PROFILE();
		(*onReceiveCallback)(&inst);
	}
		
	valueReceived= false;
}

void BaseInputSlot::setValueReceived(){
	valueReceived= true;
	if (extValueReceived)
		*extValueReceived= true;
}

} // nodes
} // clover
