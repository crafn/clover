#include "memory.hpp"
#include "util/mem_chunk.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace game {

static util::UniquePtr<util::MemChunk> g_singleFrameMemChunk;

void createMemoryPools(SizeType single_frame_mem_size){
	g_singleFrameMemChunk=
		util::makeUniquePtr<util::MemChunk>(single_frame_mem_size, "singleFrameMem");
	SingleFrameStorage::value.setMemory(g_singleFrameMemChunk.get());
}

void destroyMemoryPools(){
	SingleFrameStorage::value.setMemory(nullptr);
	g_singleFrameMemChunk.reset();
}

} // game
} // clover
