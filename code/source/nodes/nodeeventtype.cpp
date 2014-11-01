#include "nodeeventtype.hpp"

namespace clover {
namespace nodes {

NodeEventType::NodeEventType()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(argumentsAttribute, "arguments", {}){

}

NodeEventType::~NodeEventType(){
}

void NodeEventType::resourceUpdate(bool load, bool force){
	if (getResourceState() == State::Uninit || load){
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void NodeEventType::createErrorResource(){
	argumentsAttribute.get().clear();
	setResourceState(State::Error);
}

} // nodes
} // clover