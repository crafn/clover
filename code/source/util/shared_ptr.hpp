#ifndef CLOVER_UTIL_SHARED_PTR_HPP
#define CLOVER_UTIL_SHARED_PTR_HPP

#include "build.hpp"
#include "script/typestring.hpp"

#include <memory>

namespace clover {
namespace util {

/// std::shared_ptr equivalent (for script)
/// Used to instantiate no-count ref types in script
/// Usage: "SharedPtr<Type>", in script: SharedPtr<Type@>
template <typename T>
class SharedPtr {
public:
	template <typename Script>
	static void registerToScript(Script& s);
	
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

template <typename T>
template <typename Script>
void SharedPtr<T>::registerToScript(Script& s){
	using Type= SharedPtr<T>;
	
	s. template registerObjectType<Type>();
	s. template registerMethod(&Type::emplace, "emplace");
	s. template registerMethod(&Type::clear, "clear");
	s. template registerMethod<const T* (Type::*)() const>(&Type::get, "get");
	s. template registerMethod<T* (Type::*)()>(&Type::get, "get");
	s. template registerMethod<const T& (Type::*)() const>(&Type::ref, "ref");
	s. template registerMethod<T& (Type::*)()>(&Type::ref, "ref");
}

template <typename T>
struct TypeStringTraits<SharedPtr<T>> {
	/// Must use T* because T probably isn't registered to script as a value type
	static util::Str8 type(){ return "::SharedPtr<" + script::TypeString<T*>()() + ">"; }
};

} // util
} // clover

#endif // CLOVER_UTIL_SHARED_PTR_HPP
