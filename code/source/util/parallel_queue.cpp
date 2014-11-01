#include "parallel_queue.hpp"
#include "ensure.hpp"
#include "hardware/clstate.hpp"

namespace clover {
namespace util {

ParallelQueue::ParallelQueue()
		: queue(hardware::gClState->createCommandQueue(hardware::gClState->getDefaultDevice())){
}

ParallelQueue::ParallelQueue(ParallelQueue&& other):
	queue(std::move(other.queue)){
	other.queue.id= 0;
}

ParallelQueue::~ParallelQueue(){
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
	hardware::gClState->flush(queue);
}

void ParallelQueue::finish(){
	hardware::gClState->finish(queue);
}

void ParallelQueue::enqueue(const hardware::ClState::Work& w){
	hardware::gClState->enqueueWork(w, queue);
}

} // util
} // clover