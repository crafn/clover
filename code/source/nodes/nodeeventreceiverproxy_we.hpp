#ifndef CLOVER_NODES_NODEEVENTRECEIVERPROXY_WE_HPP
#define CLOVER_NODES_NODEEVENTRECEIVERPROXY_WE_HPP

#include "build.hpp"
#include "game/worldentity_handle.hpp"
#include "nodeeventreceiverproxy.hpp"

namespace clover {
namespace nodes {
	
class WeNodeEventReceiverProxy : public NodeEventReceiverProxy {
public:
	WeNodeEventReceiverProxy(const game::WeHandle& h);
	virtual ~WeNodeEventReceiverProxy();
	
	virtual void onEvent(const NodeEvent&);
	virtual WeNodeEventReceiverProxy* clone();
private:
	game::WeHandle handle;
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODEEVENTRECEIVERPROXY_WE_HPP