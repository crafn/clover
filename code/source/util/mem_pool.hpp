#ifndef CLOVER_UTIL_MEM_POOL_HPP
#define CLOVER_UTIL_MEM_POOL_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/tidy_ptr.hpp"
#include "util/mem_chunk.hpp"

namespace clover {
namespace util {

/// @return How much address should be adjusted forward in
///			memory to be correctly aligned
SizeType alignedMemAdjustment(	uint8* addr,
								SizeType alignment);

/// Base class for memory pools
/// @todo Emergency heap allocations in dev build to prevent crashes
class ENGINE_API MemPool {
public:
	MemPool()= default;
	DELETE_COPY(MemPool);
	DEFAULT_MOVE(MemPool);

	void setMemory(MemChunk* chunk);

protected:
	virtual void onMemoryAcquire(){}
	virtual void onMemoryRelease(){}

	const MemChunk& memory() const;

private:
	TidyPtr<MemChunk> memChunk;
};

} // util
} // clover

#endif // CLOVER_UTIL_MEM_POOL_HPP