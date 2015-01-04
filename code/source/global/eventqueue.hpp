#ifndef CLOVER_GLOBAL_EVENTQUEUE_HPP
#define CLOVER_GLOBAL_EVENTQUEUE_HPP

#include "event.hpp"

namespace clover {
namespace global {

/// @todo Remove this worthless class
class EventQueue {
public:
	/// Sends last and removes from queue
	bool sendLast();

	global::Event& getLast();
	void popLast();
	
	int32 size();

	/// Called by Event
	void queue(global::Event e);

private:
	util::DynArray<global::Event> events;
};

} // global
} // clover

#endif // CLOVER_GLOBAL_EVENTQUEUE_HPP
