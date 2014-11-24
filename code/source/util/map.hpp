#ifndef CLOVER_UTIL_MAP_HPP
#define CLOVER_UTIL_MAP_HPP

#include "build.hpp"
#include "hash.hpp"
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

template <typename K, typename V, typename C>
struct Hash32<Map<K, V, C>> {
	uint32 operator()(const Map<K, V, C>& value) const
	{
		uint32 h= 0;
		for (auto&& p : value)
			h += hash32(p);
		return h;
	}
};

namespace detail {

template <typename K, typename V, typename... Ts>
struct Key<std::map<K, V, Ts...>> {
	using Type= K;
};

} // detail
} // util
} // clover

#endif // CLOVER_UTIL_MAP_HPP
