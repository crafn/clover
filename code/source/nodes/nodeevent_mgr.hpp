#ifndef CLOVER_NODES_NODEEVENT_MGR_HPP
#define CLOVER_NODES_NODEEVENT_MGR_HPP

#include "build.hpp"
#include "nodeevent.hpp"

namespace clover {
namespace nodes {

class NodeEventMgr {
public:
	
	void queue(NodeEvent& e);
	void dispatch();

private:
	util::DynArray<NodeEvent> events;
};

extern NodeEventMgr* gNodeEventMgr;

} // nodes
} // clover

#endif // CLOVER_NODES_NODEEVENT_MGR_HPP