#ifndef CLOVER_NODES_NODEEVENTRECEIVERPROXY_HPP
#define CLOVER_NODES_NODEEVENTRECEIVERPROXY_HPP

#include "build.hpp"

namespace clover {
namespace nodes {

class NodeEvent;

class NodeEventReceiverProxy {
public:
	virtual ~NodeEventReceiverProxy(){}
	
	virtual void onEvent(const NodeEvent&)= 0;
	virtual NodeEventReceiverProxy* clone()= 0;
private:
};

} // nodes
} // clover

#endif // GAMELOGIC_FOUNDATIONS_NODES_NODEEVENTRECEIVERPROXY_HPP