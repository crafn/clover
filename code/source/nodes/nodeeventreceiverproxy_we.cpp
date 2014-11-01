#include "nodeeventreceiverproxy_we.hpp"
#include "game/worldentity.hpp"
#include "game/worldentity_set.hpp"

namespace clover {
namespace nodes {

WeNodeEventReceiverProxy::WeNodeEventReceiverProxy(const game::WeHandle& h)
	: handle(h){

}

WeNodeEventReceiverProxy::~WeNodeEventReceiverProxy(){
	
}

void WeNodeEventReceiverProxy::onEvent(const NodeEvent& e){
	if (handle)
		handle->onEvent(e);
}

WeNodeEventReceiverProxy* WeNodeEventReceiverProxy::clone(){
	return new WeNodeEventReceiverProxy(*this);
}

} // nodes
} // clover