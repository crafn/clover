#include "mem_pool_linear.hpp"
#include "debug/print.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace util {

void* LinearMemPool::allocate(SizeType size, SizeType alignment){
	debug_ensure(alignment > 0);
	debug_ensure(head != nullptr);

	SizeType adjustment=
		alignedMemAdjustment(head, alignment);

	uint8* adjusted_addr= head + adjustment;
	SizeType adjusted_size= size + adjustment;

	debug_ensure(adjusted_size >= size);

	if (head + adjusted_size > memory().end()){
		print(debug::Ch::General, debug::Vb::Critical,
				"util::LinearMemPool out of memory (%i): %s",
				(int32)memory().size(), memory().getTag());
		throw std::bad_alloc{};
	}

	head += adjusted_size;

	return static_cast<void*>(adjusted_addr);
}

void LinearMemPool::clear(){
	head= memory().begin();
}

void LinearMemPool::onMemoryAcquire(){
	head= memory().begin();
}

void LinearMemPool::onMemoryRelease(){
	head= nullptr;
}

} // util
} // clover
