#ifndef CLOVER_UTIL_ANY_HPP
#define CLOVER_UTIL_ANY_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/traits.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace util {

class Any {
public:
	struct BaseValue {
		virtual ~BaseValue() { }
		virtual util::UniquePtr<BaseValue> clone()= 0;
	};

	template <typename T>
	struct Value : BaseValue {
		Value(T&& v): value(std::move(v)) { }
		Value(const T& v): value(v) { }

		virtual util::UniquePtr<BaseValue> clone() override
		{ return util::UniquePtr<BaseValue>{new Value{value}}; }

		T value;
	};

	template <typename T>
	using V= Value<util::RemoveRef<T>>;

	Any()= default;
	Any(const Any& other): ptr(other.ptr ? other.ptr->clone() : nullptr) { }
	Any& operator=(const Any& other)
	{ ptr= other.ptr ? other.ptr->clone() : nullptr; return *this; }
	DEFAULT_MOVE(Any);

	template <typename T>
	Any(const T& v): ptr(util::UniquePtr<BaseValue>(new V<T>{v})) { }

	template <typename T>
	Any(T&& v): ptr(util::UniquePtr<BaseValue>(new V<T>{std::move(v)})) { }
 
	template <typename T>
	Any& operator=(const T& v)
	{ ptr= util::UniquePtr<BaseValue>(new V<T>{v}); return *this; }

	template <typename T>
	Any& operator=(T&& v)
	{ ptr= util::UniquePtr<BaseValue>(new V<T>{std::move(v)}); return *this; }

	void reset() { ptr.reset(); }
	bool empty() const { return ptr.get() == nullptr; }

private:
	template <typename T>
	friend T anyCast(const Any& any);

	util::UniquePtr<BaseValue> ptr;
};

/// @warning Won't report failures
template <typename T>
T anyCast(const Any& any)
{ return ((Any::V<T>*)any.ptr.get())->value; }

} // util
} // clover

#endif // CLOVER_UTIL_ANY_HPP
