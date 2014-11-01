#include "mem_pool.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace util {

SizeType alignedMemAdjustment(	uint8* addr,
								SizeType alignment){
	SizeType mask= (alignment - 1);
	SizeType misalignment=
		reinterpret_cast<SizeType>(addr) & mask;
	SizeType adjustment= alignment - misalignment;

	debug_ensure(misalignment <= alignment);

	if (misalignment == 0)
		return 0;
	else
		return adjustment;
}

void MemPool::setMemory(MemChunk* chunk){
	if (memChunk){
		onMemoryRelease();
		memChunk->release();
	}

	memChunk= chunk;

	if (memChunk){
		memChunk->acquire();
		onMemoryAcquire();
	}
}

const MemChunk& MemPool::memory() const {
	debug_ensure(memChunk);
	return *memChunk;
}

} // util
} // clover