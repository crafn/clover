#ifndef CLOVER_UTIL_TIDY_HPP
#define CLOVER_UTIL_TIDY_HPP

#include "build.hpp"

namespace clover {
namespace visual {

/// A value which resets itself on move
template <typename T, typename Default= T>
class Tidy {
public:
	Tidy(T init= Default{}): value(init) { }
	Tidy(Tidy&& other);
	Tidy(const Tidy&)= default;

	Tidy& operator=(Tidy&& other);
	Tidy& operator=(const Tidy&)= default;

	Tidy& operator=(T other) { value= std::move(other); return *this; }

	T& get() noexcept { return value; }
	const T& get() const noexcept { return value; }
	operator T&() noexcept { return value; }
	operator const T&() const noexcept { return value; }

private:
	T value;
};

template <typename T, typename Default>
Tidy<T, Default>::Tidy(Tidy&& other) {
	value= std::move(other.value);
	other.value= Default{};
}

template <typename T, typename Default>
Tidy<T, Default>& Tidy<T, Default>::operator=(Tidy&& other) {
	if (this != &other){
		value= std::move(other.value);
		other.value= Default{};
	}
	return *this;
}


} // visual
} // clover

#endif // CLOVER_UTIL_TIDY_HPP
