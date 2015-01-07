#ifndef UTIL_OPTIONAL_HPP
#define UTIL_OPTIONAL_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace util {

template <typename T>
class Optional {
public:
	Optional()= default;
	Optional(const Optional& other)
	{ operator=(other); }
	Optional& operator=(const Optional& other)
	{ if (other.value) value= util::makeUniquePtr<T>(*other.value); return *this; }

	DEFAULT_MOVE(Optional);

	Optional& operator=(T&& t)
	{ value.emplace(std::move(t)); return *this; }

	Optional& operator=(const T& t)
	{ value.emplace(t); return *this; }

	T* get() const { return value.get(); }
	T* operator->() const { return value.get(); }
	T& operator*() const { return value.ref(); }

	void reset() { value.reset(); }

	explicit operator bool() const { return value.get() != nullptr; }

private:
	/// @todo Remove indirection
	util::UniquePtr<T> value;
};

} // util
} // clover

#endif // UTIL_OPTIONAL_HPP
