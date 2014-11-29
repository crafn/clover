#include "mem_pool_chunk.hpp"
#include "debug/debugprint.hpp"

namespace clover {
namespace util {

ChunkMemPool::ChunkMemPool(SizeType chunk_size)
		: chunkSize(chunk_size){
	ensure(chunk_size > 0);
	// No silly alignments
	ensure(chunk_size % 2 == 0);
}

void* ChunkMemPool::allocate(SizeType size, SizeType alignment){
	if (freeChunks.empty()){
		print(debug::Ch::General, debug::Vb::Critical,
				"util::ChunkMemPool out of memory: %s", memory().getTag());
		throw std::bad_alloc{};
	}

	SizeType chunk_i= freeChunks.back();

	uint8* addr= base + chunk_i*size;
	SizeType adjustment=
		alignedMemAdjustment(addr, alignment);

	SizeType adjusted_size= size + adjustment;
	if (adjusted_size > chunkSize || alignment > chunkSize){
		print(debug::Ch::General, debug::Vb::Critical,
				"util::ChunkMemPool %s: requested allocation incompatible "
				"with chunk size: %i, %i, %i, %i",
				memory().getTag(),
				(int32)size, (int32)adjustment,
				(int32)chunkSize, (int32)alignment);
		throw std::bad_alloc{};
	}

	freeChunks.popBack();
	chunkStates[chunk_i]= chunkUsed;
	++allocatedCount;
	return static_cast<void*>(addr + adjustment);
}

void ChunkMemPool::deallocate(void* mem){
	debug_ensure(allocatedCount > 0);
	--allocatedCount;

	SizeType chunk_i= (static_cast<uint8*>(mem) - base)/chunkSize;
	debug_ensure(chunk_i < chunkCount);

	freeChunks.pushBack(chunk_i);
	chunkStates[chunk_i]= chunkFree;
}

void ChunkMemPool::onMemoryAcquire(){
	// Everything is easier if chunks are aligned to chunk sizes
	SizeType adjustment=
		alignedMemAdjustment(memory().begin(), chunkSize);

	base= memory().begin() + adjustment;

	SizeType available_mem_size= memory().end() - base;

	chunkCount= available_mem_size / chunkSize;
	freeChunks.resize(chunkCount);
	chunkStates.resize(chunkCount);
	for (SizeType i= 0; i < freeChunks.size(); ++i){
		// First chunk to be allocated is the chunk of index 0
		freeChunks[i]= freeChunks.size() - i - 1;
	}
}

void ChunkMemPool::onMemoryRelease(){
	debug_ensure(freeChunks.size() == chunkCount);

	allocatedCount= 0;

	chunkStates.clear();
	freeChunks.clear();
	chunkCount= 0;
	base= nullptr;
}

} // util
} // clover
