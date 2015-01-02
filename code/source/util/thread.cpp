#include "thread.hpp"
#include "debug/debugprint.hpp"

#include <cstring>
#include <map>

namespace clover {
namespace util {

#if THREADS == THREADS_PTHREADS

#include <pthread.h>

struct PThreadPriority {
	int schedPolicy;
	int schedPriority;
};

#if OS == OS_LINUX
// Not sure how well these match with win32 thread priorities
static const std::map<ThreadPriority, PThreadPriority> pthreadPriorities= {
	{ ThreadPriority::Idle,		PThreadPriority{ SCHED_IDLE, 0}},
	{ ThreadPriority::Low,		PThreadPriority{ SCHED_BATCH, 0}},
	{ ThreadPriority::Normal,	PThreadPriority{ SCHED_OTHER, 0}},
	{ ThreadPriority::High,		PThreadPriority{ SCHED_FIFO, 2}},
	{ ThreadPriority::Critical,	PThreadPriority{ SCHED_FIFO, 15}}	
};

#else

/// Pthreads standard scheduling policies
// Not sure how well these match with win32 thread priorities
static const std::map<ThreadPriority, PThreadPriority> pthreadPriorities= {
	{ ThreadPriority::Idle,		PThreadPriority{ SCHED_OTHER, -15}},
	{ ThreadPriority::Low,		PThreadPriority{ SCHED_OTHER, -2}},
	{ ThreadPriority::Normal,	PThreadPriority{ SCHED_OTHER, 0}},
	{ ThreadPriority::High,		PThreadPriority{ SCHED_FIFO, 2}},
	{ ThreadPriority::Critical,	PThreadPriority{ SCHED_FIFO, 15}}
};
#endif

void setThreadPriority(pthread_t handle, ThreadPriority p){
	const PThreadPriority& priority= pthreadPriorities.at(p);
	
	sched_param param;
	param.sched_priority= priority.schedPriority;
	int err= pthread_setschedparam(handle, priority.schedPolicy, &param);
	if (err != 0){
		print(debug::Ch::General, debug::Vb::Critical, "pthread_setschedparam failed: %s", strerror(err));
	}
}

void setThreadPriority(ThreadPriority p){
	setThreadPriority(pthread_self(), p);
}

#elif THREADS == THREADS_WIN32

#include <windows.h>

static const std::map<ThreadPriority, int> winPriorities= {
	{ ThreadPriority::Idle,		THREAD_PRIORITY_IDLE },
	{ ThreadPriority::Low,		THREAD_PRIORITY_LOWEST },
	{ ThreadPriority::Normal,	THREAD_PRIORITY_NORMAL },
	{ ThreadPriority::High,		THREAD_PRIORITY_HIGHEST },
	{ ThreadPriority::Critical,	THREAD_PRIORITY_TIME_CRITICAL }
};

void setThreadPriority(HANDLE handle, ThreadPriority p){
	int err= SetThreadPriority(handle, winPriorities.at(p));
	if (err == 0){
		print(debug::Ch::General, debug::Vb::Critical, "SetThreadPriority failed: %i", (int)GetLastError());
	}
}

void setThreadPriority(ThreadPriority p){
	setThreadPriority(GetCurrentThread(), p);
}

#endif

void setThreadPriority(Thread& t, ThreadPriority p){
	setThreadPriority(t.native_handle(), p);
}

} // util
} // clover
