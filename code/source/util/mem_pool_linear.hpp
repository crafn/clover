#ifndef CLOVER_UTIL_MEM_POOL_LINEAR_HPP
#define CLOVER_UTIL_MEM_POOL_LINEAR_HPP

#include "build.hpp"
#include "util/mem_pool.hpp"

namespace clover {
namespace util {

/// Memory pool for linear allocation scheme
class LinearMemPool : public MemPool {
public:
	/// Memory allocator interface
	void* allocate(SizeType size, SizeType alignment);
	void deallocate(void* mem){}

	/// Frees allocated memory
	void clear();

protected:
	virtual void onMemoryAcquire() override;
	virtual void onMemoryRelease() override;

private:
	uint8* head= nullptr;
};

} // util
} // clover

#endif // CLOVER_UTIL_MEM_POOL_LINEAR_HPP