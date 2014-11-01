#ifndef CLOVER_UTIL_ATOMIC_HPP
#define CLOVER_UTIL_ATOMIC_HPP

#include "build.hpp"
#include "util/mutex.hpp"
#include "util/tidy_ptr.hpp"

#include <algorithm>
#include <atomic>
#include <type_traits>

namespace clover {
namespace util {

/// Wrapper for std::atomic allowing copying and moving
/// @note Copying and moving aren't atomic operations
template <typename T, typename Enable= void>
class Atomic {
public:
	Atomic()= default;
	Atomic(T v): value(std::move(v)){}
	Atomic(const Atomic& other): value(other.value.load()){}
	Atomic(Atomic&& other): value(other.value.load()){}
	
	Atomic& operator=(const Atomic& other){ value= other.value.load(); return *this; }
	Atomic& operator=(Atomic&& other){ value= other.value.load(); return *this; }
	
	void store(T v){ value.store(std::move(v)); }
	T load() const { return value.load(); }
	operator T() const { return value; }
	
	/// @todo Rest of std::atomic operations
	
private:
	std::atomic<T> value;
};

template <typename T>
class Atomic<T*> : public Atomic<T*, int> {
public:
	using Base= Atomic<T*, int>;
	
	using Base::Base;
	
	T& operator*(){ return *Base::load(); }
	T* operator->(){ return Base::load(); }
};

/// Atomic wrapper for classes
template <typename T>
class Atomic<T, typename std::enable_if<	!std::is_integral<T>::value &&
											!std::is_enum<T>::value &&
											!std::is_pointer<T>::value >::type> {
public:
	template <typename U>
	class Proxy {
	public:
		
		Proxy(U& v, RecursiveMutex& m): value(&v), mutex(&m){ mutex->lock(); }
		Proxy(Proxy&&)= default;
		Proxy(const Proxy&)= delete;
		~Proxy(){ if (mutex) mutex->unlock(); }
		
		U* operator->(){ return value.get(); }
		
	private:
		TidyPtr<U> value;
		TidyPtr<RecursiveMutex> mutex;
	};
	
	Atomic()= default;
	Atomic(T v): value(std::move(v)){}
	
	Proxy<T> operator->(){ return Proxy<T>(value, access); }
	Proxy<const T> operator->() const { return Proxy<const T>(value, access); }
	
private:
	
	T value;
	mutable RecursiveMutex access;
};

} // util
} // clover

#endif // CLOVER_UTIL_ATOMIC_HPP