#ifndef CLOVER_UTIL_TRAITS_HPP
#define CLOVER_UTIL_TRAITS_HPP

#include "build.hpp"

#include <functional>
#include <type_traits>
#include <utility>

namespace clover {
namespace util {

template <typename T>
struct LambdaTraits;

template <typename T>
struct FunctionTraits : public LambdaTraits<T> {
	// T is hopefully lambda
	// Must use non-specialized struct for this because lambda type cannot be specialized
};

template <typename R, typename... Args>
struct FunctionTraits<R(Args...)> {
	static const SizeType argCount= sizeof...(Args);
	
	using Return= R;
	using Signature= R(Args...);
	
	template <SizeType N>
	struct Argument {
		typedef typename std::tuple_element<N, std::tuple<Args...>>::type Type;
	};
};

template <typename R, typename... Args>
struct FunctionTraits<std::function<R(Args...)>> : public FunctionTraits<R(Args...)> {};

template <typename R, typename... Args>
struct FunctionTraits<R(*)(Args...)> : public FunctionTraits<R(Args...)> {};

template <typename R, typename O, typename... Args>
struct FunctionTraits<R(O::*)(Args...)> {
	typedef O Class;
	typedef R Return;
	
	using Signature= R(Args...);
	using Ptr= R(O::*)(Args...);
};

// Must be after FunctionTraits
template <typename T>
struct LambdaTraits : public FunctionTraits<decltype(&T::operator())> {};

template <typename T>
struct TemplateType {
	typedef T Type;
};

template <typename R, typename O, typename... Args>
struct FunctionTraits<R(O::*)(Args...) const> {
	using Class= O;
	using Return= R;
	
	// Workaround for error "invalid qualifiers for non-member function type" which occurs with line (g++ 4.7.2)
	// using SignatureType= R(Args...) const;
	using Signature= typename TemplateType<R(Args...) const>::Type;
	using Ptr= R(O::*)(Args...) const;
};

/// Workaround for this idiotic thing:
/// Compiles:					std::function<void(void)>;
/// Invalid parameter type:		using alias= void; std::function<void(alias)>;
/// Workaround:					std::function<ToSignature<void, alias>::Type>;
template <typename R, typename... Args>
struct ToSignature {
	using Type= R(Args...);
};

template <typename R>
struct ToSignature<R, void>{
	using Type= R(void);
};

template <typename A, typename B, typename True, typename False, bool Same= std::is_same<A, B>::value>
struct IfSame;

template <typename A, typename B, typename True, typename False>
struct IfSame<A, B, True, False, true> { using Type= A; };

template <typename A, typename B, typename True, typename False>
struct IfSame<A, B, True, False, false> { using Type= B; };



template <typename T>
struct MemberPtrTraits;

template <typename T, typename O>
struct MemberPtrTraits<T O::*> {
	typedef O Class;
	typedef T Member;
};

/// @example SetParamPack<util::Set<int>, char>::Type == util::Set<char>
template <typename... T>
struct SetParamPack;

template <template<typename...> class U, typename... T, typename... NewT>
struct SetParamPack<U<T...>, NewT...> {
	using Type= U<NewT...>;
};

template <typename T>
struct HasEqualsOperator {
private:
	typedef int16 Yes;
	typedef int32 No;
	
	template <typename U> static Yes Test(decltype(&U::operator==));
	template <typename U> static No Test(...);
public:
	static const bool value= sizeof(Test<T>(0)) == sizeof(Yes);
};

template<class T>
struct IsAbstract {
  template<class U>
  static char check_sig(U (*)[1]);
  template<class U>
  static short check_sig(...);
  static constexpr bool value = sizeof(IsAbstract<T>::template check_sig<T>(0)) - 1;
};

/// @todo Separate identifier and namespaces
/// @todo Add TemplateTypeStringTraits
template <typename T>
struct TypeStringTraits; /*
	static util::Str8 type();
	static util::Str8 shortType();
};*/

template <typename T>
struct AlwaysTrue {
	static constexpr bool value = true;
};

template <typename B, typename D>
struct IsBaseOf {
private:
	using Yes= int16;
	using No= int32;
	
	static Yes test(B*);
	static No test(...);
public:
	// Should work in cases in which std::is_base_of doesn't
	// like A, B: public A, C: public B, IsBaseOf<A, C>::value == true
	static constexpr bool value= sizeof(test((D*)nullptr)) == sizeof(Yes); 
};

template <bool b>
using EnableIf= typename std::enable_if<b>::type;

template <typename T>
using RemoveConst= typename std::remove_const<T>::type;

template <typename T>
using RemoveRef= typename std::remove_reference<T>::type;

template <typename T>
using Plain= RemoveConst<RemoveRef<T>>;

template <typename T>
using RemovePtr= typename std::remove_pointer<T>::type;

template <typename T>
constexpr bool isPtr(){ return std::is_pointer<T>::value; }


} // util
} // clover

#endif // CLOVER_UTIL_TRAITS_HPP
