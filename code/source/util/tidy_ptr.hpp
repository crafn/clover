#ifndef CLOVER_UTIL_TIDY_PTR_HPP
#define CLOVER_UTIL_TIDY_PTR_HPP

#include "build.hpp"

namespace clover {
namespace util {

/// A raw pointer substitute which zeroes itself when moved from
template <typename T>
class TidyPtr {
public:
	TidyPtr(T* p= nullptr) noexcept : ptr(p){}
	TidyPtr(TidyPtr&& other) noexcept;
	TidyPtr(const TidyPtr&)= default;

	TidyPtr& operator=(TidyPtr&& other) noexcept;
	TidyPtr& operator=(const TidyPtr&)= default;

	TidyPtr& operator=(T* other) noexcept { ptr= other; return *this; }

	T* get() const noexcept { return ptr; }
	T* operator->() const noexcept { return ptr; }
	T& operator*() const noexcept { return *ptr; }

	explicit operator bool() const noexcept { return ptr != nullptr; }

private:
	T* ptr;
};

template <typename T>
TidyPtr<T>::TidyPtr(TidyPtr&& other) noexcept {
	ptr= other.ptr;
	other.ptr= nullptr;
}

template <typename T>
TidyPtr<T>& TidyPtr<T>::operator=(TidyPtr&& other) noexcept {
	if (this != &other){
		ptr= other.ptr;
		other.ptr= nullptr;
	}
	return *this;
}

} // util
} // clover

#endif // CLOVER_UTIL_TIDY_PTR_HPP
