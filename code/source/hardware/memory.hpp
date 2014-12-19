#ifndef CLOVER_HARDWARE_MEMORY_HPP
#define CLOVER_HARDWARE_MEMORY_HPP

#include "build.hpp"

/// Single object new is called by noexcept and array new operators in C++11
void* operator new(size_t size);
void operator delete(void* mem) noexcept;

namespace clover {
namespace hardware {

void createHeap(SizeType size, SizeType max_blocks);

// Allocate from heap
void* allocate(SizeType size);

// Free from heap
void deallocate(void* mem);

} // hardware
} // clover

#endif // CLOVER_HARDWARE_MEMORY_HPP
