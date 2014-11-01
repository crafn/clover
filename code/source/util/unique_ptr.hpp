#ifndef CLOVER_UTIL_UNIQUEPTR_HPP
#define CLOVER_UTIL_UNIQUEPTR_HPP

#include "build.hpp"
#include "script/typestring.hpp"

#include <memory>

namespace clover {
namespace util {

/// std::unique_ptr equivalent (for script)
/// Used to instantiate no-count ref types in script
/// Usage: "UniquePtr<Type>", in script: UniquePtr<Type@>
template <typename T>
class UniquePtr {
public:
	template <typename Script>
	static void registerToScript(Script& s);
	
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
	std::unique_ptr<T> ptr;
};

template <typename T, typename... Args>
UniquePtr<T> makeUniquePtr(Args&&... args){
	return UniquePtr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
template <typename Script>
void UniquePtr<T>::registerToScript(Script& s){
	using Type= UniquePtr<T>;
	
	s. template registerObjectType<Type>();
	s. template registerMethod(&Type::emplace<>, "emplace");
	s. template registerMethod(&Type::reset, "reset");
	s. template registerMethod(&Type::get, "get");
	s. template registerMethod(&Type::ref, "ref");
}

template <typename T>
struct TypeStringTraits<UniquePtr<T>> {
	/// Must use T* because T probably isn't registered to script as a value type
	static util::Str8 type(){ return "::UniquePtr<" + script::TypeString<T*>()() + ">"; }
};

} // util
} // clover

#endif // CLOVER_UTIL_UNIQUEPTR_HPP