#ifndef CLOVER_UTIL_KEY_HPP
#define CLOVER_UTIL_KEY_HPP

#include "build.hpp"
#include "dyn_array.hpp"

namespace clover {
namespace util {
namespace detail {

template <typename T>
struct Key;

template <typename T>
struct Key<T&> { using Type= typename Key<T>::Type; };
template <typename T>
struct Key<const T> { using Type= typename Key<T>::Type; };
template <typename T>
struct Key<const T&> { using Type= typename Key<T>::Type; };

} // detail

/// Type of key of container C
template <typename C>
using Key= typename detail::Key<C>::Type;

/// Keys of container C
template <typename C>
auto keys(const C& c)
-> DynArray<Key<C>>
{
	DynArray<Key<C>> ret;
	for (auto&& pair : c) {
		ret.pushBack(pair.first);
	}
	return ret;
}

} // clover
} // util

#endif // CLOVER_UTIL_KEY_HPP
