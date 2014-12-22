#include "nodeevent_mgr.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace nodes {

NodeEventMgr _gNodeEventMgr;
NodeEventMgr* gNodeEventMgr= &_gNodeEventMgr;

void NodeEventMgr::queue(NodeEvent& e){
	events.pushBack(std::move(e));
}

void NodeEventMgr::dispatch(){
	PROFILE();
	for (auto& m : events){
		m.send();
	}
	
	events.clear();
}

} // nodes
} // clover
