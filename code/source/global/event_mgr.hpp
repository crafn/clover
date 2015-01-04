#ifndef CLOVER_GLOBAL_EVENT_MGR_HPP
#define CLOVER_GLOBAL_EVENT_MGR_HPP

#include "util/time.hpp"
#include "event.hpp"
#include "eventqueue.hpp"

namespace clover {
namespace global {

class EventReceiver;

class EventMgr {
public:
	EventMgr();
	~EventMgr();
	
	void registerReceiver(global::EventReceiver& recv, global::Event::EType event_type);
	void unregisterReceiver(global::EventReceiver& recv, global::Event::EType event_type);
	void unregisterReceiver(global::EventReceiver& recv);
	void replaceReceiver(global::EventReceiver& old_recv, global::EventReceiver& new_recv);
	util::DynArray<global::EventReceiver*> getRegisteredReceivers(global::Event::EType event_type);

	/// Sends queued events to receivers
	void dispatch();

	void queueEvent(global::Event e);

private:
	// Automatic receivers
	util::Map<global::Event::EType, util::DynArray<global::EventReceiver*>> receiverMap;
	EventQueue queue;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_EVENT_MGR_HPP

