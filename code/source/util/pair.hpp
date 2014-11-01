#ifndef CLOVER_UTIL_PAIR_HPP
#define CLOVER_UTIL_PAIR_HPP

#include "build.hpp"
#include "key.hpp"

#include <utility>

namespace clover {
namespace util {

template <typename A, typename B>
using Pair= std::pair<A, B>;

template <typename A, typename B>
Pair<typename std::decay<A>::type, typename std::decay<B>::type>
makePair(A&& a, B&& b){
	return std::make_pair(std::forward<A>(a), std::forward<B>(b));
}

namespace detail {

template <typename K, typename V>
struct Key<std::pair<K, V>> {
	using Type= K;
};

} // detail
} // util
} // clover

#endif // CLOVER_UTIL_PAIR_HPP
