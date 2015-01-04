#include "env.hpp"
#include "eventreceiver.hpp"
#include "event_mgr.hpp"

namespace clover {
namespace global {

EventReceiver::EventReceiver()
		: listenCount(0){
}

EventReceiver::EventReceiver(EventReceiver&& other)
		: listenCount(0){
	global::g_env->eventMgr->replaceReceiver(other, *this);
}

EventReceiver::~EventReceiver(){
	if (listenCount > 0)
		global::g_env->eventMgr->unregisterReceiver(*this);
}

void EventReceiver::listenForEvent(global::Event::EType t){
	global::g_env->eventMgr->registerReceiver(*this, t);
	++listenCount;
}

void EventReceiver::unlistenForEvent(global::Event::EType t){
	ensure(listenCount > 0);
	global::g_env->eventMgr->unregisterReceiver(*this, t);
	--listenCount;
}

} // global
} // clover
