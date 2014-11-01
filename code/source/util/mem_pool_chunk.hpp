#ifndef CLOVER_UTIL_MEM_POOL_CHUNK_HPP
#define CLOVER_UTIL_MEM_POOL_CHUNK_HPP

#include "build.hpp"
#include "util/dyn_bitarray.hpp"
#include "util/dyn_array.hpp"
#include "util/mem_pool.hpp"

namespace clover {
namespace util {

/// Memory pool for equally sized chunks
class ChunkMemPool : public MemPool {
public:
	ChunkMemPool(SizeType chunk_size);

	/// Memory allocator interface
	/// Allocated size can't be greater than specified in constructor
	void* allocate(SizeType size, SizeType alignment);
	void deallocate(void* mem);

	void* getPtr(SizeType i) const { return base + chunkSize*i; }
	bool isUsed(SizeType i) const { return chunkStates[i] == chunkUsed; }
	SizeType getAllocatedCount() const { return allocatedCount; }
	SizeType getChunkCount() const { return chunkCount; }
	SizeType getChunkSize() const { return chunkSize; }

protected:
	virtual void onMemoryAcquire() override;
	virtual void onMemoryRelease() override;

private:
	static constexpr bool chunkUsed= true;
	static constexpr bool chunkFree= false;

	SizeType chunkSize;

	uint8* base= nullptr;
	SizeType chunkCount= 0;
	util::DynArray<SizeType> freeChunks;
	DynBitArray chunkStates;

	SizeType allocatedCount= 0;
};

} // util
} // clover

#endif // CLOVER_UTIL_MEM_POOL_CHUNK_HPP