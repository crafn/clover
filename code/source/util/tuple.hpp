#ifndef CLOVER_UTIL_TUPLE_HPP
#define CLOVER_UTIL_TUPLE_HPP

#include "build.hpp"
#include "hash.hpp"

#include <tuple>

namespace clover {
namespace util {

template <typename... Args>
using Tuple= std::tuple<Args...>;

template <SizeType N, typename T>
using TupleElement= std::tuple_element<N, T>;

template <typename... Args>
auto makeTuple(Args&&... args)
-> decltype(std::make_tuple(std::forward<Args>(args)...)){
	return std::make_tuple(std::forward<Args>(args)...);
}

// Helper for calculating tuple hash
template <SizeType N, typename... Args>
struct TupleHash32 {
	uint32 operator()(const Tuple<Args...>& tuple) const {
		// Multiplying so that order of tuple parameters matter
		return (N+1)*Hash32< typename TupleElement<N, Tuple<Args...>>::type >()(std::get<N>(tuple)) + TupleHash32<N-1, Args...>()(tuple);
	}
};

template <typename... Args>
struct TupleHash32<0, Args...> {
	uint32 operator()(const Tuple<Args...>& tuple) const {
		return Hash32< typename TupleElement<0, Tuple<Args...>>::type >()(std::get<0>(tuple));
	}
};

/// Hash for Tuple
template <typename... Args>
struct Hash32<Tuple<Args...>> {
	uint32 operator()(const Tuple<Args...>& tuple) const {
		return TupleHash32<sizeof...(Args) - 1, Args...>()(tuple);
	}
};

} // util
} // clover

#endif // CLOVER_UTIL_TUPLE_HPP
