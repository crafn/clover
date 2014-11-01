#include "profiling.hpp"
#include "profiler.hpp"

namespace clover {
namespace util {
namespace detail {

BlockInfo BlockProfiler::createBlockInfo(const char* func, SizeType line, const char* label){
	return BlockInfo{func, line, label};
}

BlockProfiler::BlockProfiler(BlockInfo& block){
	Profiler::onBlockEnter(block);
}

BlockProfiler::~BlockProfiler(){
	Profiler::onBlockExit();
}

StackJoiner::StackJoiner(){
	Profiler::onStackJoin();
}

StackJoiner::~StackJoiner(){
	Profiler::onStackDetach();
}

StackDetacher::StackDetacher(){
	Profiler::onStackDetach();
}

StackDetacher::~StackDetacher(){
	Profiler::onStackJoin();
}

void setSuperThread(std::thread::id super_id){
	Profiler::setSuperThread(super_id);
}

} // detail
} // util
} // clover