#ifndef CLOVER_UTIL_SET_HPP
#define CLOVER_UTIL_SET_HPP

#include "build.hpp"
#include "hash.hpp"
#include <set>

namespace clover {
namespace util {

template <typename T>
using Set= std::set<T>;

template <typename T>
struct Hash32<Set<T>> {
	uint32 operator()(const Set<T>& value) const
	{
		uint32 h= 0;
		for (auto&& m : value)
			h += hash32(m);
		return h;
	}
};

} // util
} // clover

#endif // CLOVER_UTIL_SET_HPP
