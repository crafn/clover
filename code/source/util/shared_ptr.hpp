#ifndef CLOVER_UTIL_SHARED_PTR_HPP
#define CLOVER_UTIL_SHARED_PTR_HPP

#include "build.hpp"

#include <memory>

namespace clover {
namespace util {

/// std::shared_ptr equivalent (for script)
/// Used to instantiate no-count ref types in script
/// Usage: "SharedPtr<Type>", in script: SharedPtr<Type@>
template <typename T>
class SharedPtr {
public:
	SharedPtr()= default;
	SharedPtr(T* value): ptr(value){}
	
	void emplace(){ ptr= std::shared_ptr<T>(new T()); }
	void clear(){ ptr.reset(); }
	
	const T* get() const { return ptr.get(); }
	T* get(){ return ptr.get(); }
	const T& ref() const { ensure(ptr.get()); return *ptr.get(); }
	T& ref() { ensure(ptr.get()); return *ptr.get(); }
	
	const T* operator->() const { return get(); }
	T* operator->(){ return get(); }
	
	const T& operator*() const { return ref(); }
	T& operator*(){ return ref(); }
	
private:
	std::shared_ptr<T> ptr;
};


template <typename T, typename... Args>
SharedPtr<T> makeSharedPtr(Args&&... args)
{ return SharedPtr<T>(new T(std::forward<Args>(args)...)); }

} // util
} // clover

#endif // CLOVER_UTIL_SHARED_PTR_HPP
