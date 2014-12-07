#include "parallel_queue.hpp"
#include "ensure.hpp"
#include "hardware/clstate.hpp"

namespace clover {
namespace util {

ParallelQueue::ParallelQueue(){
	ensure(hardware::gClState);
	queue= hardware::gClState->createCommandQueue(hardware::gClState->getDefaultDevice());
}

ParallelQueue::ParallelQueue(ParallelQueue&& other):
	queue(std::move(other.queue)){
	other.queue.id= 0;
}

ParallelQueue::~ParallelQueue(){
	ensure(hardware::gClState);
	if (queue.id){
		hardware::gClState->destroyCommandQueue(queue);
	}
}

ParallelQueue& ParallelQueue::operator=(ParallelQueue&& other){
	queue= std::move(other.queue);
	other.queue.id= 0;

	return *this;
}

void ParallelQueue::flush(){
	ensure(hardware::gClState);
	hardware::gClState->flush(queue);
}

void ParallelQueue::finish(){
	ensure(hardware::gClState);
	hardware::gClState->finish(queue);
}

void ParallelQueue::enqueue(const hardware::ClState::Work& w){
	ensure(hardware::gClState);
	hardware::gClState->enqueueWork(w, queue);
}

} // util
} // clover