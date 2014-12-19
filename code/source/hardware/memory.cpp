#include "memory.hpp"
#include "util/profiling.hpp"

#include <array>
#include <cassert>
#include <cstdlib>
#include <thread>
#include <mutex>

#define OVERRIDE_DEFAULT_NEW 0

// Memory used before actual heap creation
#define INIT_MEM_SIZE 1024*1024*50
#define INIT_MEM_MAX_BLOCKS 1024*10

void* operator new(size_t size)
{ return clover::hardware::allocate(size); }

void operator delete(void* mem) noexcept
{ clover::hardware::deallocate(mem); }

namespace clover {
namespace hardware {
namespace detail {

struct MemBlock {
	uint8* base= nullptr;
	SizeType size= 0;
	bool free= false;

	uint8* end() const { return base + size; }
};

/// Unfinished heap implementation (really slow and probably buggy)
/// @todo Buddy/slab allocation to speed up
class Heap {
public:
	void create(SizeType heap_size, SizeType max_blocks){
		if (isValid())
			throw std::bad_alloc{};

		void* mem= std::malloc(heap_size);
		if (!mem)
			throw std::bad_alloc{};

		size= heap_size;

		// Space for tracking blocks is taken from beginning of the heap
		blocks= static_cast<MemBlock*>(mem);
		blockCount= 0;
		maxBlockCount= max_blocks;

		storageBase= base() + maxBlockCount*sizeof(MemBlock);

		if (storageBase >= base() + size)
			throw std::bad_alloc{};

		if (maxBlockCount == 0)
			throw std::bad_alloc{};

		// Create initial free block
		blocks[0].base= storageBase;
		blocks[0].size= storageSize();
		blocks[0].free= true;
		blockCount= 1;
		freeBlockFinder= &blocks[0];
	}

	void destroy(){
		std::free(base());
		size= 0;
		blocks= nullptr;
	}

	void* allocate(SizeType size){
		if (size == 0)
			throw std::bad_alloc{};

		++allocCounter;

		SizeType block_i= findFreeBlock(size);
		void* mem= allocateFromBlock(block_i, size);

		return mem;
	}

	void deallocate(void* mem){
		SizeType block_i= findAllocatedBlock(mem);
		deallocateBlock(block_i);
	}

	bool contains(void* mem) const noexcept {
		return mem >= base() && mem < base() + size;
	}

	bool isValid() const noexcept { return base() != nullptr; }

private:
	uint8* base(){ return reinterpret_cast<uint8*>(blocks); }
	const uint8* base() const { return reinterpret_cast<const uint8*>(blocks); }
	SizeType storageSize() const { return size - (storageBase - base()); }

	SizeType findFreeBlock(SizeType size){
		/// @todo Alignment
		for (SizeType i= 0; i < blockCount; ++i){
			if (freeBlockFinder->free && freeBlockFinder->size >= size)
				return freeBlockFinder - &blocks[0];
			++freeBlockFinder;
			if (freeBlockFinder >= blocks + blockCount)
				freeBlockFinder= &blocks[0];
		}

		throw std::bad_alloc{};
	}

	SizeType findAllocatedBlock(void* mem) const {
		/// @todo Better search algorithm
		for (SizeType i= 0; i < blockCount; ++i){
			if (blocks[i].base == mem)
				return i;
		}

		throw std::bad_alloc{};
	}

	uint8* allocateFromBlock(SizeType block_i, SizeType size){
		assert(size <= blocks[block_i].size);
		/// @todo Alignment

		SizeType wasted_bytes= blocks[block_i].size - size;

		if (wasted_bytes > 1){
			divideBlock(block_i, size);
		}

		blocks[block_i].free= false;
		return blocks[block_i].base;
	}

	void deallocateBlock(SizeType block_i){
		if (block_i >= blockCount)
			throw std::bad_alloc{};

		blocks[block_i].free= true;

		mergeAround(block_i);
	}

	void divideBlock(SizeType block_i, SizeType new_size){
		SizeType old_size= blocks[block_i].size;
		SizeType created_block_size= old_size - new_size;
		assert(old_size > new_size);

		addBlock();
		assert(block_i + 1 < blockCount);
		std::memmove(
				&blocks[block_i + 1],
				&blocks[block_i],
				sizeof(MemBlock)*(blockCount - block_i - 1));

		blocks[block_i].size= new_size;
		blocks[block_i + 1].base= blocks[block_i].end();
		blocks[block_i + 1].size= created_block_size;

		assert(blocks[block_i].size + blocks[block_i + 1].size == old_size);
	}

	void mergeAround(SizeType block_i){
		assert(blocks[block_i].free);

		if (block_i > 0 && blocks[block_i - 1].free){
			mergeBlocks(block_i - 1);
			--block_i;
		}

		if (block_i + 1 < blockCount && blocks[block_i + 1].free){
			mergeBlocks(block_i);
		}
	}

	void mergeBlocks(SizeType first_i){
		assert(blockCount > 0);

		SizeType second_i= first_i + 1;
		blocks[first_i].size += blocks[second_i].size;

		std::memmove(
				&blocks[second_i],
				&blocks[second_i + 1],
				sizeof(MemBlock)*(blockCount - second_i - 1));

		--blockCount;
		if ((SizeType)(freeBlockFinder - &blocks[0]) >= blockCount)
			freeBlockFinder= &blocks[0];
	}

	void addBlock(){
		if (blockCount >= maxBlockCount)
			throw std::bad_alloc{};

		++blockCount;
	}

	bool isCorrupt() const {
		SizeType total_mem= 0;
		uint8* prev_base= nullptr;
		uint8* prev_end= nullptr;
		for (SizeType i= 0; i < blockCount; ++i){
			total_mem += blocks[i].size;

			bool condition1= contains(blocks[i].base);
			bool condition2= contains(blocks[i].end() - 1);
			bool condition3= prev_end <= blocks[i].base;
			bool condition4= prev_base < blocks[i].base;
			bool condition5= blocks[i].size > 0;
			assert(condition1);
			assert(condition2);
			assert(condition3);
			assert(condition4);
			assert(condition5);

			if (	!condition1 || !condition2 || !condition3 ||
					!condition4 || !condition5)
				return true;

			prev_base= blocks[i].base;
			prev_end= blocks[i].end();
		}
		
		assert(total_mem == storageSize());
		return total_mem != storageSize();
	}

	SizeType size= 0;
	uint8* storageBase= nullptr;

	MemBlock* blocks= nullptr;
	SizeType blockCount= 0;
	SizeType maxBlockCount= 0;

	MemBlock* freeBlockFinder= nullptr;

	SizeType allocCounter= 0;

};

static Heap g_initHeap;
static Heap g_heap;
std::mutex g_memMutex;

} // detail
using namespace detail;

void createHeap(SizeType size, SizeType max_allocations)
{
#if OVERRIDE_DEFAULT_NEW
	std::lock_guard<std::mutex> g(g_memMutex);
	g_heap.create(size, max_allocations);
#endif
}

void* allocate(SizeType size)
{
	util::profileSystemMemAlloc();
#if OVERRIDE_DEFAULT_NEW
	std::lock_guard<std::mutex> g(g_memMutex);
	if (g_heap.isValid()) {
		return g_heap.allocate(size);
	} else {
		if (!g_initHeap.isValid())
			g_initHeap.create(INIT_MEM_SIZE, INIT_MEM_MAX_BLOCKS);

		return g_initHeap.allocate(size);
	}
#else
	return std::malloc(size);
#endif
}

void deallocate(void* mem)
{
#if OVERRIDE_DEFAULT_NEW
	std::lock_guard<std::mutex> g(g_memMutex);
	if (!mem)
		return;

	if (g_initHeap.contains(mem))
		g_initHeap.deallocate(mem);
	else
		g_heap.deallocate(mem);
#else
	std::free(mem);
#endif
}

} // hardware
} // clover
