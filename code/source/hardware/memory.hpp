#ifndef CLOVER_HARDWARE_MEMORY_HPP
#define CLOVER_HARDWARE_MEMORY_HPP

#include "build.hpp"

/// Single object new is called by noexcept and array new operators in C++11
void* operator new(size_t size);
void operator delete(void* mem) noexcept;

namespace clover {
namespace hardware {

// Heap can't be created at first allocation, because e.g. MinGW does some
// allocations before main which are released by msvcrt free(), resulting to crash
void createHeap();

// Allocate from heap
void* heapAllocate(SizeType size);

// Free from heap
void heapDeallocate(void* mem);

} // hardware
} // clover

#endif // CLOVER_HARDWARE_MEMORY_HPP
