#include "baseoutputslot.hpp"
#include "baseinputslot.hpp"

namespace clover {
namespace nodes {

BaseOutputSlot::BaseOutputSlot(SignalType t)
		: BaseSlot(t){
}

BaseOutputSlot::~BaseOutputSlot(){
}

void BaseOutputSlot::attach(SubSignalType from, SubSignalType to, BaseInputSlot& input){
	inputs.pushBack({&input, from, to});
}
	
} // nodes
} // clover