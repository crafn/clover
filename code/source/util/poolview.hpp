#ifndef CLOVER_UTIL_POOLVIEW_HPP
#define CLOVER_UTIL_POOLVIEW_HPP

#include "build.hpp"
#include "util/ensure.hpp"
#include "util/mem_pool_chunk.hpp"

namespace clover {
namespace util {

/// View for memory pool
/// @note Make sure that every type allocated in the pool is T
template <typename T>
class PoolView {
public:
	PoolView(ChunkMemPool& pool)
			: pool(&pool){
		debug_ensure(pool.getChunkSize() == sizeof(T));
	}

	T* operator[](SizeType i) const {
		return static_cast<T*>(pool->isUsed(i) ? pool->getPtr(i) : nullptr);
	}

	SizeType allocatedCount() const { return pool->getAllocatedCount(); }
	SizeType size() const { return pool->getChunkCount(); }

private:
	ChunkMemPool* pool;
};

} // util
} // clover

#endif // CLOVER_UTIL_POOLVIEW_HPP