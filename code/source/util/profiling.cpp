#include "profiling.hpp"
#include "profiler.hpp"

namespace clover {
namespace util {
namespace detail {

BlockInfo BlockProfiler::createBlockInfo(const char* func, uint32 line, const char* label)
{ return BlockInfo{func, line, label, 0, 0}; }

BlockProfiler::BlockProfiler(BlockInfo& block)
{ Profiler::onBlockEnter(block); }

BlockProfiler::~BlockProfiler()
{ Profiler::onBlockExit(); }

StackJoiner::StackJoiner()
{ Profiler::onStackJoin(); }

StackJoiner::~StackJoiner()
{ Profiler::onStackDetach(); }

StackDetacher::StackDetacher()
{ Profiler::onStackDetach(); }

StackDetacher::~StackDetacher()
{ Profiler::onStackJoin(); }

void setSuperThread(std::thread::id super_id)
{ Profiler::setSuperThread(super_id); }

} // detail

static bool g_profilingStarted= false;
void tryEnableProfiling()
{
#if PROFILING_ENABLED
	g_profilingStarted= true;
#endif
}

#if PROFILING_ENABLED
void profileSystemMemAlloc()
{
	if (g_profilingStarted)
		Profiler::onSystemMemAlloc();
}
#endif

} // util
} // clover
