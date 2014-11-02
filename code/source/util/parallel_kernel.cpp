#include "parallel_kernel.hpp"
#include "parallel_queue.hpp"

namespace clover {
namespace util {

ParallelKernel::ParallelKernel(hardware::ClState::Kernel&& kernel_):
	attachedQueue(0),
	kernel(std::move(kernel_)){

}

void ParallelKernel::enqueue(const Work& w){
	ensure(attachedQueue);
	if (w.globalSize == 0)
		return;

	hardware::ClState::Work work;
	work.globalSize= w.globalSize;
	work.localSize= 1;

	/// @todo Find most optimal value for localSize at runtime
	// 32 is somewhat optimal for nvidia
	for (int i= 32; i > 0; --i){
		if (work.globalSize % i == 0)
			work.localSize= i;
	}

	work.kernel= kernel;

	attachedQueue->enqueue(work);
}

template <>
void ParallelKernel::setArgument(uint32 arg_id, util::ParallelBuffer& b, uint32 count){
	ensure(count == 1 && attachedQueue);
	hardware::gClState->setKernelArgument(kernel, arg_id, b.getDId(), count);
	b.attachToQueue(*attachedQueue);
}

} // util
} // clover