#ifndef CLOVER_GLOBAL_EVENTRECEIVER_HPP
#define CLOVER_GLOBAL_EVENTRECEIVER_HPP

#include "build.hpp"
#include "event.hpp"

namespace clover {
namespace global {

class BaseEventForwarder;

class EventReceiver {
public:
	EventReceiver();
	/// @todo Implement listen-state copying
	EventReceiver(const EventReceiver&)= default;
	EventReceiver(EventReceiver&&);	
	virtual ~EventReceiver();

	EventReceiver& operator=(const EventReceiver&)= default;
	EventReceiver& operator=(EventReceiver&&)= default;

	virtual void onEvent(global::Event&){};

	void listenForEvent(global::Event::EType t);
	void unlistenForEvent(global::Event::EType t);
	
private:
	uint32 listenCount;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_EVENTRECEIVER_HPP