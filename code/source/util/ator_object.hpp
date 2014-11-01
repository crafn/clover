#ifndef CLOVER_UTIL_ATOR_OBJECT_HPP
#define CLOVER_UTIL_ATOR_OBJECT_HPP

#include "build.hpp"
#include "util/ensure.hpp"

#include <type_traits>

namespace clover {
namespace util {

/// STL compatible allocator which uses memory allocator
template <typename T, typename MemAtor>
class ObjectAtor {
public:
	using value_type= T;

	ObjectAtor(MemAtor& ator)
			: ator(&ator){
	}

	DEFAULT_COPY(ObjectAtor);
	DEFAULT_MOVE(ObjectAtor);

	template <typename U, typename A>
	ObjectAtor(const ObjectAtor<U, A>& other)
			: ator(other.ator){
	}

	T* allocate(SizeType count){
		debug_ensure(ator);

		void* mem= ator->allocate(	count*sizeof(T),
									std::alignment_of<T>::value);
		return static_cast<T*>(mem);
	}

	void deallocate(T* mem, SizeType count){
		ator->deallocate(static_cast<void*>(mem));
	}

	bool operator==(const ObjectAtor& other) const {
		return ator == other.ator;
	}

private:
	MemAtor* ator;
};

} // util
} // clover

#endif // CLOVER_UTIL_ATOR_OBJECT_HPP