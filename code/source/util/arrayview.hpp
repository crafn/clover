#ifndef CLOVER_UTIL_ARRAYVIEW_HPP
#define CLOVER_UTIL_ARRAYVIEW_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/ensure.hpp"
#include "util/traits.hpp"

namespace clover {
namespace util {

/// Non-owning slice of contiguous data
/// Use this instead of 'const std::vector<T>&', because
/// that causes problems with different allocators
template <typename T>
class ArrayView {
public:
	ArrayView(T* begin, T* end)
			: beginPtr(begin)
			, endPtr(end){
	}

	ArrayView(T* ptr, SizeType size)
			: beginPtr(ptr)
			, endPtr(ptr + size){
	}

	DEFAULT_COPY(ArrayView);
	DEFAULT_MOVE(ArrayView);

	T* begin() const { return beginPtr; }
	T* end() const { return endPtr; }

	T& operator[](SizeType i) const {
		debug_ensure(i < size() || (i == 0 && size() == 0));
		return beginPtr[i];
	}

	bool empty() const { return beginPtr == endPtr; }
	SizeType size() const { return endPtr - beginPtr; }

	operator ArrayView<const T>() const {
		return ArrayView<const T>{beginPtr, endPtr};
	}

	template <typename U= T, typename= EnableIf<isPtr<U>()>>
	operator ArrayView<const RemovePtr<T>*>() const {
		return ArrayView<const RemovePtr<T>*>{beginPtr, endPtr};
	}

	template <typename U= T, typename= EnableIf<isPtr<U>()>>
	operator ArrayView<const RemovePtr<T>* const>() const {
		return ArrayView<const RemovePtr<T>* const>{beginPtr, endPtr};
	}

private:
	T* beginPtr;
	T* endPtr;
};

/// @todo begin & end version
template <typename T>
auto asArrayView(const T& t) ->
decltype(ArrayView<RemoveRef<decltype(*t.data())>>(t.data(), t.size())){
	return ArrayView<RemoveRef<decltype(*t.data())>>(t.data(), t.size());
}

} // util
} // clover

#endif // CLOVER_UTIL_ARRAYVIEW_HPP
