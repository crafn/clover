#ifndef CLOVER_UTIL_DYNAMIC_HPP
#define CLOVER_UTIL_DYNAMIC_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/unique_ptr.hpp"
#include "script/typestring.hpp"

#include <type_traits>

namespace clover {
namespace util {

/// Makes type to be instantiated dynamically. Useful with e.g. memory pools
/// Used to instantiate no-count ref types in script
/// @usage Dynamic<Type>, in script: Dynamic<Type@>
template <typename T>
class Dynamic {
public:
	template <typename Script>
	static void registerToScript(Script& s);

	/// @todo Add variadic constructor

	Dynamic()
			: value(util::makeUniquePtr<T>()){
	}

	template<	typename U= T,
				class= typename std::enable_if<std::is_copy_constructible<U>::value>::type>
	Dynamic(const Dynamic& other)
			: value(util::makeUniquePtr<T>(*other)){
	}

	template<	typename U= T,
				class= typename std::enable_if<std::is_copy_assignable<U>::value>::type>
	Dynamic& operator=(const Dynamic& other){
		*value= *other.value;
		return *this;
	}

	DEFAULT_MOVE(Dynamic);

	const T& get() const { return *value; }
	T& get() { return *value; }

	/// Pointer semantics (without the ability of being nullptr)
	const T* operator->() const { return value.get(); }
	T* operator->(){ return value.get(); }

	const T& operator*() const { return get(); }
	T& operator*(){ return get(); }

	operator const T& () const { return get(); }
	operator T& () { return get(); }

private:
	UniquePtr<T> value;
};

template <typename T>
template <typename Script>
void Dynamic<T>::registerToScript(Script& s){
	using Type= Dynamic<T>;

	s. template registerObjectType<Type>();
	s. template registerMethod<const T& (Type::*)() const>(&Type::get, "get");
	s. template registerMethod<T& (Type::*)()>(&Type::get, "get");
}

template <typename T>
struct TypeStringTraits<util::Dynamic<T>> {
	/// Must use T* because T probably isn't registered to script as a value type
	static util::Str8 type(){ return "::Dynamic<" + script::TypeString<T*>()() + ">"; }
};

} // util
} // clover

#endif // CLOVER_UTIL_DYNAMIC_HPP
