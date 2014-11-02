#ifndef CLOVER_SCRIPT_TYPESTRING_HPP
#define CLOVER_SCRIPT_TYPESTRING_HPP

#include "build.hpp"
#include "util.hpp"
#include "util/ensure.hpp"
#include "util/string.hpp"
#include "util/traits.hpp"

namespace clover {
namespace util {

// Defined here because type string stuff not used anywhere else
template <>
struct TypeStringTraits<int8> {
	static util::Str8 type(){ return "::int8"; }
};

template <>
struct TypeStringTraits<int32> {
	static util::Str8 type(){ return "::int32"; }
	static util::Str8 shortType(){ return "i"; }
};

template <>
struct TypeStringTraits<int64> {
	static util::Str8 type(){ return "::int64"; }
	static util::Str8 shortType(){ return "l"; }
};

template <>
struct TypeStringTraits<uint32> {
	static util::Str8 type(){ return "::uint32"; }
	static util::Str8 shortType(){ return "u"; }
};

template <>
struct TypeStringTraits<uint64> {
	static util::Str8 type(){ return "::uint64"; }
	static util::Str8 shortType(){ return "ul"; }
};


template <>
struct TypeStringTraits<real32> {
	static util::Str8 type(){ return "::real32"; }
	static util::Str8 shortType(){ return "f"; }
};

template <>
struct TypeStringTraits<real64> {
	static util::Str8 type(){ return "::real64"; }
	static util::Str8 shortType(){ return "d"; }
};

template <>
struct TypeStringTraits<bool> {
	static util::Str8 type(){ return "::bool"; }
	static util::Str8 shortType(){ return "b"; }
};

template <>
struct TypeStringTraits<void> {
	static util::Str8 type(){ return "::void"; }
};

template <typename T, SizeType N>
struct TypeStringTraits<RealVector<T, N>> {
	static util::Str8 type(){ return util::Str8::format("::Vec%i%s", (int32)N, TypeStringTraits<T>::shortType().cStr()); }
	static util::Str8 shortType(){ return util::Str8::format("%i%s", (int32)N, TypeStringTraits<T>::shortType().cStr()); }
};

template <typename T, SizeType N>
struct TypeStringTraits<IntegerVector<T, N>> {
	static util::Str8 type(){ return util::Str8::format("::Vec%i%s", (int32)N, TypeStringTraits<T>::shortType().cStr()); }
	static util::Str8 shortType(){ return util::Str8::format("%i%s", (int32)N, TypeStringTraits<T>::shortType().cStr()); }
};

template <typename R, typename T>
struct TypeStringTraits<util::RtTransform<R,T>> {
	// It's assumed that R and T has the same underlying pod types (e.g. util::Vec2d, real64 or util::Vec3f, real32)
	static util::Str8 type(){ return util::Str8::format("::RtTransform%s", TypeStringTraits<T>::shortType().cStr()); }
};

template <typename S, typename R, typename T>
struct TypeStringTraits<util::SrtTransform<S,R,T>> {
	static util::Str8 type(){ return util::Str8::format("::SrtTransform%s", TypeStringTraits<T>::shortType().cStr()); }
};

template <typename T>
struct TypeStringTraits<Quaternion<T>> {
	static util::Str8 type(){ return util::Str8::format("::Quat%s", TypeStringTraits<T>::shortType().cStr()); }
};

} // util
namespace script {

/// Used to create script-compatible strings from all kind of types
struct BaseTypeString {
	util::Str8 name, prefix, suffix;

	explicit BaseTypeString(const util::Str8& name_):
		name(name_) {}

	/// Returns declaration of type
	util::Str8 operator()() const {
		util::Str8 ret;

		if (!prefix.empty()) ret += prefix + " ";
		ret += getType();
		if (!suffix.empty()) ret += " " + suffix;
		if (!name.empty()) ret += " " + name;

		return (ret);
	}

	virtual util::Str8 getType() const = 0;

	/// Angelscript funcdef name compatible string
	virtual util::Str8 getFuncdefStr() const { return Utils::leadingNamespaceRemoved(getType()); }

	BaseTypeString& asParam(){
		return *this;
	}
};


// bool = std::is_enum probably not needed anymore
template <typename T, bool = std::is_enum<T>::value>
struct TypeString : public BaseTypeString {
	#define DEFAULT_TYPESTRING_CONSTRUCTOR() \
	explicit TypeString(const util::Str8& name_ = ""): \
		BaseTypeString(name_) {}

	DEFAULT_TYPESTRING_CONSTRUCTOR()

	util::Str8 getType() const {
		return (util::TypeStringTraits<T>::type());
	}

};

/// Enum
template <typename T>
struct TypeString<T, true> : public BaseTypeString {
	DEFAULT_TYPESTRING_CONSTRUCTOR()

	util::Str8 getType() const {
		return (util::TypeStringTraits<T>::type());
	}

};

/// Const
template <typename T>
struct TypeString<const T, false> : public TypeString<T> {
	using Base= TypeString<T>;

	explicit TypeString(const util::Str8& name_ = ""):
		TypeString<T>(name_) {
		this->prefix= "const";
	}

	virtual util::Str8 getFuncdefStr() const { return "c_" + Base::getFuncdefStr(); }
};



/// Pointer
template <typename T>
struct TypeString<T*, false> : public TypeString<T> {
	using Base= TypeString<T>;

	explicit TypeString(const util::Str8& name_ = ""):
		TypeString<T>(name_) {
		this->suffix= "@" + this->suffix;
	}

	virtual util::Str8 getFuncdefStr() const { return "ptr_" + Base::getFuncdefStr(); }

};

/// Const Pointer
template <typename T>
struct TypeString<const T*, false> : public TypeString<T*> {
	using Base= TypeString<T*>;

	explicit TypeString(const util::Str8& name_ = ""):
		TypeString<T*>(name_) {
		this->prefix= "const";
	}

	virtual util::Str8 getFuncdefStr() const override { return "c" + Base::getFuncdefStr(); }
};

/// Reference
template <typename T>
struct TypeString<T&, false> : public TypeString<T> {
	using Base= TypeString<T>;

	explicit TypeString(const util::Str8& name_ = ""):
		TypeString<T>(name_) {
		this->suffix= this->suffix + "&";
	}

	virtual util::Str8 getFuncdefStr() const override { return "ref_" + Base::getFuncdefStr(); }
};

/// Const Reference
template <typename T>
struct TypeString<const T&, false> : public TypeString<T&> {
	using Base= TypeString<T&>;

	explicit TypeString(const util::Str8& name_ = ""):
		TypeString<T&>(name_){
		this->prefix= "const";
	}

	virtual util::Str8 getFuncdefStr() const override { return "c" + Base::getFuncdefStr(); }

	TypeString& asParam(){
		this->suffix= this->suffix + " in";
		return *this;
	}
};

/// Meta-class which converts variadic template list to script-function argument string
template <SizeType N>
struct ArgumentStringComposer {
	template <typename Arg, typename... RestOfArgs>
	static util::Str8 getString(const util::Str8& head= ""){
		return	std::move(
					ArgumentStringComposer<N-1>::template getString<RestOfArgs...>(
						head + TypeString<Arg>().asParam()() + ", "
					)
				);
	}

};

template <>
struct ArgumentStringComposer<1> {
	template <typename Arg, typename...>
	static util::Str8 getString(const util::Str8& head= ""){
		return head + TypeString<Arg>().asParam()();
	}
};

template <>
struct ArgumentStringComposer<0> {
	template <typename...>
	static util::Str8 getString(const util::Str8& head= ""){
		return head;
	}
};

/// Meta-class which converts e.g. (char, const int&) -> char_const_ref_int
template <SizeType N>
struct FuncdefArgumentStringComposer {
	template <typename Arg, typename... RestOfArgs>
	static util::Str8 getString(const util::Str8& head= ""){
		return	FuncdefArgumentStringComposer<N-1>::template getString<RestOfArgs...>(
						head + TypeString<Arg>().getFuncdefStr() + "_");
	}

};

template <>
struct FuncdefArgumentStringComposer<1> {
	template <typename Arg, typename...>
	static util::Str8 getString(const util::Str8& head= ""){
		return head + TypeString<Arg>().getFuncdefStr();
	}
};

template <>
struct FuncdefArgumentStringComposer<0> {
	template <typename...>
	static util::Str8 getString(const util::Str8& head= ""){
		return head;
	}
};


/// Function
template <typename R, typename... Args>
struct TypeString<R(Args...), false> {
	util::Str8 name;

	explicit TypeString(const util::Str8& name_= ""):
		name(name_) {}

	util::Str8 operator()() const {
		/// @todo Additional return values to out -arguments
		util::Str8 ret;
		constexpr SizeType arg_count= sizeof...(Args);
		ret += TypeString<R>()() + " " + name + "("
			+ ArgumentStringComposer<arg_count>::template getString<Args...>()
			+ ")";

		return (ret);
	}

	/// E.g. void (int, int) -> f_void_int_int
	util::Str8 getFuncdefStr() const {
		constexpr SizeType arg_count= sizeof...(Args);
		return	TypeString<R>().getFuncdefStr() + "_" +
				FuncdefArgumentStringComposer<arg_count>::template getString<Args...>();
	}

	util::Str8 lastParamDiscardedStr() const {
		/// @todo Additional return values to out -arguments
		util::Str8 ret;
		constexpr SizeType arg_count= sizeof...(Args);
		ret += TypeString<R>()() + " " + name + "("
			+ ArgumentStringComposer<arg_count-1>::template getString<Args...>()
			+ ")";

		return (ret);
	}

private:
};

/// Const function
template <typename R, typename... Args>
struct TypeString<R(Args...) const, false> {
	util::Str8 name;
	explicit TypeString(const util::Str8& name_):
		name(name_) {}

	util::Str8 operator()() const {
		/// @todo Additional return values to out -arguments
		util::Str8 ret;
		constexpr SizeType arg_count= sizeof...(Args);
		ret += TypeString<R>()() + " " + name + "("
			+ ArgumentStringComposer<arg_count>::template getString<Args...>()
			+ ") const";

		return (ret);
	}

	/// E.g. void (int, int) const -> funcdef_void_int_int_const
	util::Str8 getFuncdefStr() const {
		return TypeString<R(Args...)>().getFuncdefStr() + "_const";
	}

	util::Str8 lastParamDiscardedStr() const {
		/// @todo Additional return values to out -arguments
		util::Str8 ret;
		constexpr SizeType arg_count= sizeof...(Args);
		ret += TypeString<R>()() + " " + name + "("
			+ ArgumentStringComposer<arg_count-1>::template getString<Args...>()
			+ ") const";

		return (ret);
	}
private:
};
	
} // script
namespace util {

template <typename T, template <typename> class Alloc, typename T2>
struct TypeStringTraits<util::DynArray<T, Alloc, T2>> {
	static util::Str8 type(){ return "::Array<" + script::TypeString<T>()() + ">"; }
};

template <typename T>
struct TypeStringTraits<std::function<T>>{
	static util::Str8 type(){ return "std::function_" + script::TypeString<T>().getFuncdefStr(); }
};

} // util
} // clover

#endif // CLOVER_SCRIPT_TYPESTRING_HPP