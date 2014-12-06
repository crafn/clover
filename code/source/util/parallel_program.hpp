#ifndef CLOVER_UTIL_PARALLEL_PROGRAM_HPP
#define CLOVER_UTIL_PARALLEL_PROGRAM_HPP

#include "build.hpp"
#include "hardware/clstate.hpp"
#include "string.hpp"
#include "dyn_array.hpp"
#include "parallel_kernel.hpp"
#include "util/linkedlist.hpp"

namespace clover {
namespace util {

/// If false, using util::Parallel* stuff will crash
bool parallelProcessingSupported();

class ParallelQueue;
class ParallelProgram {
public:
	typedef int32 KernelId;

	enum BufferFlag {
		ReadWrite= hardware::ClState::ReadWrite,
		WriteOnly= hardware::ClState::WriteOnly,
		ReadOnly= hardware::ClState::ReadOnly,
		UseHostPtr= hardware::ClState::UseHostPtr,
		AllocHostPtr= hardware::ClState::AllocHostPtr,
		CopyHostPtr= hardware::ClState::CopyHostPtr
	};

	/// Loads and compiles program
	ParallelProgram(util::Str8 path);
	
	ParallelProgram(const ParallelProgram&)= delete;
	ParallelProgram(ParallelProgram&&);

	ParallelProgram();
	virtual ~ParallelProgram();

	ParallelProgram& operator=(ParallelProgram&&);
	ParallelProgram& operator=(const ParallelProgram&)= delete;

	void attachToQueue(util::ParallelQueue& q);

	void compile(util::Str8 path);

	util::ParallelKernel& createKernel(util::Str8 kernelname);

private:
	hardware::ClState::Program program;
	util::LinkedList<util::ParallelKernel> kernels;
	util::ParallelQueue* attachedQueue;
};

} // util
} // clover

#endif // CLOVER_UTIL_PARALLEL_PROGRAM_HPP