#ifndef CLOVER_UTIL_THREAD_HPP
#define CLOVER_UTIL_THREAD_HPP

#include "build.hpp"

#include <thread>

namespace clover {
namespace util {

using Thread= std::thread;

enum class ThreadPriority {
	Idle,
	Low,
	Normal,
	High,
	Critical
};

/// Sets priority of calling thread
void setThreadPriority(ThreadPriority p);

/// Sets priority of thread t
void setThreadPriority(Thread& t, ThreadPriority p);

} // util
} // clover

#endif // CLOVER_UTIL_THREAD_HPP
