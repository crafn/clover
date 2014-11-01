#include "baseslot.hpp"

namespace clover {
namespace nodes {

BaseSlot::BaseSlot(SignalType t)
		: type(t){
}

BaseSlot::~BaseSlot(){
}

SignalType BaseSlot::getType() const {
	return type;
}

} // nodes
} // clover