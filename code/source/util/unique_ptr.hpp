#ifndef CLOVER_UTIL_UNIQUEPTR_HPP
#define CLOVER_UTIL_UNIQUEPTR_HPP

#include "build.hpp"
#include "ensure.hpp"

#include <memory>

namespace clover {
namespace util {

/// std::unique_ptr equivalent (for script)
/// Used to instantiate no-count ref types in script
/// Usage: "UniquePtr<Type>", in script: UniquePtr<Type@>
template <typename T>
class UniquePtr {
public:
	UniquePtr()= default;
	UniquePtr(T* value): ptr(value){}

	template <typename... Args>
	void emplace(Args&&... args){
		ptr= std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}
	void reset(){ ptr.reset(); }

	T* get() const { return ptr.get(); }
	T& ref() const { ensure(ptr.get()); return *ptr.get(); }

	T* operator->() const { return get(); }
	T& operator*() const { return ref(); }

	explicit operator bool() const { return ptr.get() != nullptr; }

private:
	/// @todo Replace with custom impl, because std::unique_ptr is broken;
	///       Ptr can't be accessed during dtor -- with new & delete it can.
	std::unique_ptr<T> ptr;
};

template <typename T, typename... Args>
UniquePtr<T> makeUniquePtr(Args&&... args){
	return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

} // util
} // clover

#endif // CLOVER_UTIL_UNIQUEPTR_HPP
