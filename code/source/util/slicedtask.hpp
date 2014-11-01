#ifndef CLOVER_UTIL_SLICEDTASK_HPP
#define CLOVER_UTIL_SLICEDTASK_HPP

#include "build.hpp"
#include "util/mutex.hpp"

#include <future>
#include <functional>

namespace clover {
namespace util {

/// A task which can be executed synchronously in multiple passes
/// @warning Task is executed in another thread so e.g. opengl won't like
/// @todo Fibers would be nice
class SlicedTask {
public:
	/// Pause-point for task
	using Yield= std::function<void ()>;
	static Yield nullYield(){ return [](){}; }
	static constexpr real64 eternity= -1.0;

	using TaskFunc= std::function<void (const SlicedTask::Yield&)>;

	SlicedTask(TaskFunc f);
	
	void runFor(real64 seconds);

	bool isFinished() const;
	
	/// Relaunches task to be completed
	void relaunch();
	
private:
	void launchFuture();
	void callTask();

	TaskFunc taskFunc;

	using InverseLock= InverseLockGuard<Mutex>;
	using Lock= LockGuard<Mutex>;

	std::future<void> future;

	bool finished= false;
	bool skipYield= false;
	bool blocking= false;
	
	mutable Mutex mutex;
	Lock initialPauseLock; // Must be destroyed before future
};

} // util
} // clover

#endif // CLOVER_UTIL_SLICEDTASK_HPP