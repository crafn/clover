#include "slicedtask.hpp"
#include "debug/debugprint.hpp"
#include "util/ensure.hpp"
#include "util/profiling.hpp"
#include "util/time.hpp"

#include <chrono>

namespace clover {
namespace util {

SlicedTask::SlicedTask(TaskFunc f)
		: taskFunc(f)
		, initialPauseLock(mutex){
	launchFuture();
}

void SlicedTask::runFor(real64 seconds){
	ensure(seconds >= 0.0 || seconds == eternity);
	ensure(!finished);

	if (!blocking){
		if (seconds >= 0.0){
			auto time= util::asMicroseconds(seconds);

			{ InverseLock g(mutex); // Unleash the task thread
				if (future.wait_for(time) == std::future_status::ready){
					finished= true;
					if (future.valid())
						future.get(); // Possible exception
				}
			}
		}
		else if (seconds == eternity) {
			skipYield= true;
			{ InverseLock g(mutex);
				finished= true;
				future.get();
			}
			skipYield= false;
		}
	}
	else {
		// Single threaded
		taskFunc(nullYield());
		finished= true;
	}
}

bool SlicedTask::isFinished() const {
	return finished;
}

void SlicedTask::relaunch(){
	ensure(isFinished());
	finished= false;
	skipYield= false;
	blocking= false;
	launchFuture();
}

void SlicedTask::launchFuture(){
	std::thread::id super_id= std::this_thread::get_id();
	try {
		future= std::async(std::launch::async, [this, super_id] (){
			PROFILER_SUPER_THREAD(super_id);
			callTask();
		});
	}
	catch (std::system_error& e){
		print(debug::Ch::General, debug::Vb::Critical, "SlicedTask::launchFuture(): Launching async task failed: %s", e.what());
		print(debug::Ch::General, debug::Vb::Moderate, "SlicedTask::launchFuture(): Blocking mode enabled");
		blocking= true;
	}
}

void SlicedTask::callTask(){
	Yield yielder= [this] () {
		if (skipYield)
			return;

		/// @todo Measure time and filter unnecessary yields away

		PROFILER_STACK_DETACH();
		InverseLock g(mutex); // Unleash the main thread
		// This is needed on Linux, otherwise main thread won't get the
		// mutex and task will be completed at once. Seems like scheduling issue
		std::this_thread::sleep_for(std::chrono::nanoseconds(0));
	};
	
	{ Lock g(mutex); // Wait for running the task
		PROFILER_STACK_JOIN();
		taskFunc(yielder);
	}
}

} // util
} // clover