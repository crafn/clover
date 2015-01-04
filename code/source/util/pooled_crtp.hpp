#ifndef CLOVER_UTIL_POOLED_CRTP_HPP
#define CLOVER_UTIL_POOLED_CRTP_HPP

#include "build.hpp"
#include "util/mem_pool_chunk.hpp"
#include "util/poolview.hpp"

namespace clover {
namespace util {

/// Requires function `pool` to avoid static state
//// Requires function `pool` to avoid static state
template <typename T, util::ChunkMemPool& (*pool)()>
class PooledCrtp {
public:
	using View= PoolView<T>;

	static SizeType pooledCount() { return pool().getAllocatedCount(); }
	static View poolView() { return View(pool()); }

	static void* operator new(size_t size) { return allocate(size); }
	static void operator delete(void* mem) { deallocate(mem); }

	/// Placement versions

	static void* operator new(size_t size, void* mem){ return mem; }
	static void* operator new[](size_t size, void* mem){ return mem; }

private:
	/// Can't allocate arrays of pooled objects
	static void* operator new[](size_t size);
	static void operator delete[](void* mem);

	static void* allocate(SizeType size)
	{ return pool().allocate(size, std::alignment_of<T>::value); }

	static void deallocate(void* mem)
	{ pool().deallocate(mem); }
};

} // util
} // clover

#endif // CLOVER_UTIL_POOLED_CRTP_HPP
