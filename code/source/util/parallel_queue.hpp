#ifndef CLOVER_UTIL_PARALLER_QUEUE_HPP
#define CLOVER_UTIL_PARALLER_QUEUE_HPP

#include "build.hpp"
#include "hardware/clstate.hpp"

namespace clover {
namespace util {

class ParallelKernel;
class ParallelBuffer;

class ParallelQueue {
public:
	ParallelQueue();
	ParallelQueue(const ParallelQueue&)= delete;
	ParallelQueue(ParallelQueue&&);
	virtual ~ParallelQueue();

	ParallelQueue& operator=(ParallelQueue&&);
	ParallelQueue& operator=(const ParallelQueue&)= delete;
	
	void flush();
	void finish();

protected:
	friend class util::ParallelKernel;
	friend class util::ParallelBuffer;
	
	void enqueue(const hardware::ClState::Work& w);

private:
	hardware::ClState::CommandQueue queue;
};

} // util
} // clover

#endif // CLOVER_UTIL_PARALLER_QUEUE_HPP