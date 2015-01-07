#include "debug/print.hpp"
#include "env.hpp"
#include "event.hpp"
#include "event_mgr.hpp"
#include "eventreceiver.hpp"
#include "util/profiling.hpp"
#include "game/worldentity_mgr.hpp"

namespace clover {
namespace global {

EventMgr::EventMgr()
{
	if (!global::g_env->eventMgr)
		global::g_env->eventMgr= this;
}

EventMgr::~EventMgr()
{
	if (global::g_env->eventMgr == this)
		global::g_env->eventMgr= nullptr;
}

void EventMgr::registerReceiver(global::EventReceiver& recv, global::Event::EType event_type){
	PROFILE_("events");
	receiverMap[event_type].pushBack(&recv);
}

void EventMgr::unregisterReceiver(global::EventReceiver& recv, global::Event::EType event_type){
	PROFILE_("events");
	auto& receivers= receiverMap[event_type];
	
	for(auto it=receivers.begin(); it!=receivers.end(); ++it){
		if (*it == &recv){
			receivers.erase(it);
			break;
		}
	}
}

void EventMgr::unregisterReceiver(global::EventReceiver& recv){
	PROFILE_("events");
	for(auto& event_array_pair : receiverMap){
		for(auto it=event_array_pair.second.begin(); it!=event_array_pair.second.end(); ++it){
			if (*it == &recv){
				event_array_pair.second.erase(it);
				break;
			}
		}
	}
}

void EventMgr::replaceReceiver(global::EventReceiver& old_recv, global::EventReceiver& new_recv){
	PROFILE_("events");
	for(auto& event_array_pair : receiverMap){
		for(auto it= event_array_pair.second.begin(); it != event_array_pair.second.end(); ++it){
			if (*it == &old_recv){
				
				old_recv.unlistenForEvent(event_array_pair.first);
				new_recv.listenForEvent(event_array_pair.first);
				
				replaceReceiver(old_recv, new_recv);
				
				break;
			}
		}
	}
}

util::DynArray<global::EventReceiver*> EventMgr::getRegisteredReceivers(global::Event::EType event_type){
	PROFILE_("events");
	return receiverMap[event_type];
}

void EventMgr::dispatch(){
	PROFILE_("events");
	while(queue.size()){
		global::Event& e= queue.getLast();
		e.send();
		queue.popLast();
	}
}

void EventMgr::queueEvent(global::Event e)
{ queue.queue(e); }

} // global
} // clover
