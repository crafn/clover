#include "memory.hpp"
#include "util/profiling.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <mutex>

#define OVERRIDE_DEFAULT_NEW 1

void* operator new(size_t size)
{ return clover::hardware::heapAllocate(size); }

void operator delete(void* mem) noexcept
{ clover::hardware::heapDeallocate(mem); }

namespace clover {
namespace hardware {

/// @todo use util/ensure.hpp's fail when it doesn't allocate anymore
void fail(const char* msg)
{
	std::printf("Memory failure: %s\n", msg);
	std::abort();
}

// http://stackoverflow.com/questions/3272424/compute-fast-log-base-2-ceiling
int ceil_log2(unsigned long long x)
{
  static const unsigned long long t[6] = {
    0xFFFFFFFF00000000ull,
    0x00000000FFFF0000ull,
    0x000000000000FF00ull,
    0x00000000000000F0ull,
    0x000000000000000Cull,
    0x0000000000000002ull
  };

  int y = (((x & (x - 1)) == 0) ? 0 : 1);
  int j = 32;
  int i;

  for (i = 0; i < 6; i++) {
    int k = (((x & t[i]) == 0) ? 0 : j);
    y += k;
    x >>= k;
    j >>= 1;
  }

  return y;
}

#if OVERRIDE_DEFAULT_NEW

static const SizeType megabyte= 1024*1024;
static std::mutex g_memMutex;
static const SizeType firstBlockSize= 32;
static const SizeType blockPoolCount= 14;
// Experimental values
static const SizeType memPerBlockPool[blockPoolCount]= {
	megabyte*20, // 0
	megabyte*80, // 1
	megabyte*90, // 2
	megabyte*150, // 3
	megabyte*40, // 4
	megabyte*20, // 5
	megabyte*10, // 6
	megabyte*80, // 7
	megabyte*50, // 8
	megabyte*350, // 9
	megabyte*40, // 10
	megabyte*10, // 11
	megabyte*20, // 12
	megabyte*150, // 13
};

enum class AllocStatus : char {
	free= 0,
	alloc
};

struct BlockPool {
	uint32 allocCount;
	uint32 blockSize;
	uint32 blockCount;
	char* beginAddr;
	char* endAddr;
	uint32 nextBlock;
	AllocStatus* allocStatus; // AllocStatus[blockCount]
};

static BlockPool blockPools[blockPoolCount];
static char* blockPoolMem;
static SizeType blockPoolMemSize;
static int64 outsidePoolAllocs;

inline
bool blockPoolsInitialized() { return blockPools[0].allocStatus != nullptr; }

static
void printPoolStats()
{
	for (SizeType i= 0; i < blockPoolCount; ++i) {
		std::printf("pool[%i], %i: %i/%i = %.2f\n",
				(int)i,
				(int)blockPools[i].blockSize,
				(int)blockPools[i].allocCount,
				(int)blockPools[i].blockCount,
				(float)blockPools[i].allocCount/blockPools[i].blockCount);
	}
	std::printf("outside: %i\n", (int)outsidePoolAllocs);
}

/// @todo deinit
static
void initBlockPools()
{
	blockPoolMemSize= 0;
	for (SizeType i= 0; i < blockPoolCount; ++i)
		blockPoolMemSize += memPerBlockPool[i];
	blockPoolMem= (char*)std::malloc(blockPoolMemSize);

	SizeType block_size= firstBlockSize;
	char* block_begin_addr= blockPoolMem;
	for (SizeType i= 0; i < blockPoolCount; ++i) {
		auto& pool= blockPools[i];
		pool.allocCount= 0;
		pool.blockSize= block_size;
		pool.blockCount= memPerBlockPool[i]/block_size;
		pool.beginAddr= block_begin_addr;
		pool.endAddr= block_begin_addr + memPerBlockPool[i];
		pool.nextBlock= 0;
		pool.allocStatus= (AllocStatus*)std::calloc(pool.blockCount, sizeof(*pool.allocStatus));

		if (pool.beginAddr + pool.blockSize*pool.blockCount > blockPoolMem + blockPoolMemSize)
			fail("blockPool init failed");

		std::memset(pool.beginAddr, 0x95, pool.blockSize*pool.blockCount);

		block_begin_addr += memPerBlockPool[i];
		block_size *= 2;
	}
}

#endif

void* heapAllocate(SizeType size)
{
	if (size == 0)
		return nullptr;

	util::profileSystemMemAlloc();

#if OVERRIDE_DEFAULT_NEW
	std::lock_guard<std::mutex> g(g_memMutex);

	if (!blockPoolsInitialized())
		initBlockPools();

	volatile SizeType pool_i= ceil_log2(size);
	if (size < firstBlockSize)
		pool_i= 0;
	else
		pool_i -= ceil_log2(firstBlockSize);

	if (pool_i >= blockPoolCount) {
		++outsidePoolAllocs;
		return std::malloc(size);
	} else {
		// Allocate from pool
		auto& pool= blockPools[pool_i];
		//std::printf("alloc[%i]: %i\n", (int)pool_i, (int)size);
		if (pool.blockCount <= pool.allocCount) {
			printPoolStats();
			fail("Heap pool out of memory");
		}

		while (pool.allocStatus[pool.nextBlock] != AllocStatus::free)
			pool.nextBlock= (pool.nextBlock + 1) % pool.blockCount;

		pool.allocStatus[pool.nextBlock]= AllocStatus::alloc;
		++pool.allocCount;
		return pool.beginAddr + pool.nextBlock*pool.blockSize;
	}

#else
	return std::malloc(size);
#endif
}

void heapDeallocate(void* mem)
{
#if OVERRIDE_DEFAULT_NEW
	if (!mem)
		return;

	std::lock_guard<std::mutex> g(g_memMutex);
	if (!blockPoolsInitialized())
		initBlockPools();

	if (mem < blockPoolMem || mem >= blockPoolMem + blockPoolMemSize) {
		--outsidePoolAllocs;
		std::free(mem);
	} else {
		SizeType pool_i= blockPoolCount;

		for (SizeType i= 0; i < blockPoolCount; ++i) {
			if (mem < blockPools[i].endAddr) {
				pool_i= i;
				break;
			}
		}
		if (pool_i >= blockPoolCount)
			fail("Invalid pool_i");
		auto& pool= blockPools[pool_i];
		SizeType block_i= ((char*)mem - pool.beginAddr)/pool.blockSize;
		if (block_i > pool.blockCount)
			fail("Invalid block_i");
		pool.allocStatus[block_i]= AllocStatus::free;
		--pool.allocCount;
	}
#else
	std::free(mem);
#endif
}

} // hardware
} // clover
