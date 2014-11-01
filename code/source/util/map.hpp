#ifndef CLOVER_UTIL_MAP_HPP
#define CLOVER_UTIL_MAP_HPP

#include "build.hpp"
#include "key.hpp"

#include <map>

namespace clover {
namespace util {

template <typename Key, typename Value, typename Comparator= std::less<Key>>
using Map= std::map<Key, Value, Comparator>;

template <typename Key, typename Value, typename Comparator= std::less<Key>>
using MultiMap= std::multimap<Key, Value, Comparator>;

/// Returns a MultiMap with switched key and value
/// Comparator C2 for MultiMap must be given
template <typename C2, typename K, typename V, typename C1>
MultiMap<V, K, C2> flippedMap(const Map<K, V, C1>& map){
	MultiMap<V, K, C2> flipped;
	for (const auto& pair : map){
		flipped.emplace(pair.second, pair.first);
	}
	return flipped;
}

namespace detail {

template <typename K, typename V, typename... Ts>
struct Key<std::map<K, V, Ts...>> {
	using Type= K;
};

} // detail
} // util
} // clover

#endif // CLOVER_UTIL_MAP_HPP
