#ifndef CLOVER_UTIL_DYN_POOL_HPP
#define CLOVER_UTIL_DYN_POOL_HPP

#include "build.hpp"
#include "dyn_array.hpp"

namespace clover {
namespace util {

/// Dynamic pool container which allows tight packing, fast insertion and removal,
/// and permanent indices for elements
/// @todo Alignment
template <typename T>
class DynPool {
public:
	using This= DynPool<T>;

	~DynPool();

	SizeType add(const T& t);
	void remove(SizeType index);

	/// @return nullptr if there's no element at index
	T* find(SizeType index);
	const T* find(SizeType index) const;

	T& get(SizeType index){ return *NONULL(find(index)); }
	const T& get(SizeType index) const { return *NONULL(find(index)); }

	SizeType size() const { return elements.size(); }
	SizeType capacity() const { return elements.capacity(); }

private:
	SizeType acquireFree();
	T* asPtr(SizeType i) const;

	struct Element { char bytes[sizeof(T)]; bool used= false; };

	util::DynArray<Element> elements;
	util::DynArray<SizeType> freeIndices;
};

template <typename T>
DynPool<T>::~DynPool(){
	for (SizeType i= 0; i < size(); ++i){
		if (find(i))
			remove(i);
	}
}

template <typename T>
SizeType DynPool<T>::add(const T& t){
	SizeType i= acquireFree();
	new (asPtr(i)) T(t);
	return i;
}

template <typename T>
void DynPool<T>::remove(SizeType index){
	T* ptr= asPtr(index);
	debug_ensure(ptr != nullptr);
	ptr->~T();
	elements[index].used= false;
	/// @todo Shrink elements if many freeIndices
	freeIndices.pushBack(index);
}

template <typename T>
T* DynPool<T>::find(SizeType index){
	return const_cast<T*>(static_cast<const This&>(*this).find(index));
}

template <typename T>
const T* DynPool<T>::find(SizeType index) const {
	debug_ensure(index < size());
	return asPtr(index);
}

template <typename T>
SizeType DynPool<T>::acquireFree(){
	if (freeIndices.empty()){
		elements.pushBack(Element());
		elements.back().used= true;
		return elements.size() - 1;
	}
	else {
		SizeType i= freeIndices.back();
		elements[i].used= true;
		freeIndices.popBack();
		return i;
	}
}

template <typename T>
T* DynPool<T>::asPtr(SizeType i) const {
	debug_ensure(i < size());
	if (elements[i].used){
		return reinterpret_cast<T*>(const_cast<char*>(elements[i].bytes));
	}
	else {
		return nullptr;
	}
}

} // util
} // clover

#endif // CLOVER_UTIL_DYN_POOL_HPP