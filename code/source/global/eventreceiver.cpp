#include "eventreceiver.hpp"
#include "event_mgr.hpp"

namespace clover {
namespace global {

EventReceiver::EventReceiver()
		: listenCount(0){
}

EventReceiver::EventReceiver(EventReceiver&& other)
		: listenCount(0){
	global::gEventMgr->replaceReceiver(other, *this);
}

EventReceiver::~EventReceiver(){
	if (listenCount > 0)
		global::gEventMgr->unregisterReceiver(*this);
}

void EventReceiver::listenForEvent(global::Event::EType t){
	global::gEventMgr->registerReceiver(*this, t);
	++listenCount;
}

void EventReceiver::unlistenForEvent(global::Event::EType t){
	ensure(listenCount > 0);
	global::gEventMgr->unregisterReceiver(*this, t);
	--listenCount;
}

} // global
} // clover