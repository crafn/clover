#include "baseinputslot.hpp"
#include "debug/debugprint.hpp"

namespace clover {
namespace nodes {
	
BaseInputSlot::BaseInputSlot(SignalType t)
	: BaseSlot(t)
	, valueReceived(false){
}

BaseInputSlot::~BaseInputSlot(){
}

void BaseInputSlot::setOnReceiveCallback(const CallbackType& c){
	onReceiveCallback= c;
}

void BaseInputSlot::update(){
	if (valueReceived && onReceiveCallback)
		onReceiveCallback();
		
	valueReceived= false;
}

void BaseInputSlot::setValueReceived(){
	valueReceived= true;
	if (extValueReceived)
		*extValueReceived= true;
}

} // nodes
} // clover