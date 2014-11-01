#ifndef CLOVER_UTIL_MUTEX_HPP
#define CLOVER_UTIL_MUTEX_HPP

#include "build.hpp"

#include <thread>
#include <mutex>

namespace clover {
namespace util {

using Mutex= std::mutex;
using RecursiveMutex= std::recursive_mutex;

template <typename T>
using LockGuard= std::lock_guard<T>;

template <typename T>
class InverseLockGuard {
public:
	InverseLockGuard(T& mtx): mutex(mtx){ mutex.unlock(); }
	~InverseLockGuard(){ mutex.lock(); }
	
private:
	T& mutex;
};

} // util
} // clover

#endif // CLOVER_UTIL_MUTEX_HPP