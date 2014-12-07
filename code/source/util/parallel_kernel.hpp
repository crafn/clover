#ifndef CLOVER_UTIL_PARALLEL_KERNEL_HPP
#define CLOVER_UTIL_PARALLEL_KERNEL_HPP

#include "build.hpp"
#include "hardware/clstate.hpp"
#include "util/ensure.hpp"
#include "util/parallel_buffer.hpp"

namespace clover {
namespace util {

class ParallelQueue;
class ParallelProgram;

class ParallelKernel {
public:
	struct Work {
		int32 globalSize;
	};

	ParallelKernel(hardware::ClState::Kernel&& kernel);
	ParallelKernel(const ParallelKernel&)= delete;
	ParallelKernel(ParallelKernel&&)= default;

	ParallelKernel& operator=(ParallelKernel&&)= default;
	ParallelKernel& operator=(const ParallelKernel&)= delete;

	void attachToQueue(util::ParallelQueue& q){ attachedQueue= &q; }

	template <typename T>
	void setArgument(uint32 arg_id, T &arg, uint32 count=1);

	void enqueue(const Work& w);

private:
	friend class util::ParallelProgram;

	util::ParallelQueue* attachedQueue;

	// Program manages
	hardware::ClState::Kernel kernel;
};

template <>
void ParallelKernel::setArgument(uint32 arg_id, util::ParallelBuffer& b, uint32 count);

template <typename T>
void ParallelKernel::setArgument(uint32 arg_id, T &arg, uint32 count){
	ensure(hardware::gClState);
	hardware::gClState->setKernelArgument(kernel, arg_id, arg, count);
}

} // util
} // clover

#endif // CLOVER_UTIL_PARALLEL_KERNEL_HPP