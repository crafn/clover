#ifndef CLOVER_UTIL_ATOR_BOUD_HPP
#define CLOVER_UTIL_ATOR_BOUD_HPP

#include "build.hpp"
#include "util/ator_object.hpp"

namespace clover {
namespace util {

/// STL compatible allocator which binds allocator and storage together
/// @param Tag acts as a pool instance identifier
/// @usage	struct Tag;
///			using Storage= BoundAtorStorage<LinearMemPool, Tag>;
///			template <typename T> using Ator= BoundAtor<T, Storage>;
///			util::DynArray<int, Ator> array; array.pushBack(1);
template <	typename T,
			typename BoundStorage>
class BoundAtor {
public:
	using value_type= T;

	BoundAtor()
			: ator(BoundStorage::value){}

	DEFAULT_COPY(BoundAtor);
	DEFAULT_MOVE(BoundAtor);

	template <typename U, typename S>
	BoundAtor(const BoundAtor<U, S>& other)
			: ator(other.ator){
	}

	T* allocate(SizeType count){
		return ator. template allocate(count);
	}

	void deallocate(T* mem, SizeType count){
		ator. template deallocate(mem, count);
	}

	bool operator==(const BoundAtor& other) const {
		return ator. template operator==(other.ator);
	}

	static typename BoundStorage::Value& storage(){ return BoundStorage::value; }

private:
	ObjectAtor<T, typename BoundStorage::Value> ator;
};

/// Used with BoundAtor
template <typename MemStorage, typename Tag>
struct BoundAtorStorage {
	using Value= MemStorage;
	static Value value;
};

template <typename MemStorage, typename Tag>
MemStorage BoundAtorStorage<MemStorage, Tag>::value;

} // util
} // clover

#endif // CLOVER_UTIL_ATOR_BOUD_HPP
