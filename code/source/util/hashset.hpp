#ifndef CLOVER_UTIL_HASHSET_HPP
#define CLOVER_UTIL_HASHSET_HPP

#include "build.hpp"

#include <unordered_set>

namespace clover {
namespace util {

template <typename T>
using HashSet= std::unordered_set<T>;

} // util
} // clover

#endif // CLOVER_UTIL_HASHSET_HPP