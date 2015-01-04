#include "eventqueue.hpp"

namespace clover {
namespace global {

bool EventQueue::sendLast(){
	if (events.size() == 0)
		throw global::Exception("EventQueue::sendLast(): zero events in queue");

	if (events.back().getReceiverCount() == 0)return false; //Ei ketään kenelle lähettää

	events.back().send();

	events.popBack();

	return true;
}

global::Event& EventQueue::getLast(){
	return events.back();
}

void EventQueue::popLast(){
	events.popBack();
}

void EventQueue::queue(global::Event e){
	events.pushBack(e);
}

int32 EventQueue::size(){
	return events.size();
}

} // global
} // clover
