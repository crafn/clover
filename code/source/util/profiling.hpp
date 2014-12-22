#ifndef CLOVER_UTIL_PROFILING_HPP
#define CLOVER_UTIL_PROFILING_HPP

#include "build.hpp"
#include "util/preproc_join.hpp"

#include <thread>

// No profiling on windows due to odd crashes with mingw 4.8.2
#define PROFILING_ENABLED (ATOMIC_PTR_READWRITE == true)

namespace clover {
namespace util {
namespace detail {

struct BlockInfo {
	const char* funcName;
	uint32 line;
	/// Don't assume that labels with same string will point to
	/// the same memory - depends on compiler optimizations
	const char* label;
	uint64 exclusiveMemAllocs;
	uint64 inclusiveMemAllocs;
};

struct BlockProfiler {
	static BlockInfo createBlockInfo(	const char* func,
										uint32 line,
										const char* label);
	BlockProfiler(BlockInfo& info);
	~BlockProfiler();
};

struct StackJoiner {
	StackJoiner();
	~StackJoiner();
};

struct StackDetacher {
	StackDetacher();
	~StackDetacher();
};

void setSuperThread(std::thread::id super_id);

} // detail

/// Call in main() -- we don't want to see any impl defined pre-main stuff
void tryEnableProfiling();

#if PROFILING_ENABLED
/// Should be called on system memory allocation
void profileSystemMemAlloc();
#else
void profileSystemMemAlloc() {};
#endif

} // util
} // clover

#if PROFILING_ENABLED

/// Same as PROFILE but with a label
#define PROFILE_(label)\
	static util::detail::BlockInfo JOIN(profiler_block_info_, __LINE__)= \
		util::detail::BlockProfiler::createBlockInfo( \
				__PRETTY_FUNCTION__, \
				__LINE__, \
				label); \
	util::detail::BlockProfiler JOIN(profiler_, __LINE__)(JOIN(profiler_block_info_, __LINE__))

/// Marks current block to be profiled
#define PROFILE()\
	PROFILE_(nullptr)

/// Notifies profiler of a super thread
#define PROFILER_SUPER_THREAD(thread_id)\
	util::detail::setSuperThread(thread_id)

/// Joins callstacks of this and super thread in current scope
#define PROFILER_STACK_JOIN()\
	util::detail::StackJoiner JOIN(stack_joiner_, __LINE__)
/// Detaches callstacks of this and super thread in current scope
#define PROFILER_STACK_DETACH()\
	util::detail::StackDetacher JOIN(stack_detacher_, __LINE__)

#else

#define PROFILE_(label)
#define PROFILE()
#define PROFILER_SUPER_THREAD(thread_id)
#define PROFILER_STACK_JOIN()
#define PROFILER_STACK_DETACH()

#endif

#endif // CLOVER_UTIL_PROFILING_HPP
