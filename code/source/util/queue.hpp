#ifndef CLOVER_UTIL_QUEUE_HPP
#define CLOVER_UTIL_QUEUE_HPP

#include "build.hpp"

#include <queue>

namespace clover {
namespace util {

template <typename T>
using Queue = std::queue<T>;

} // util
} // clover

#endif // CLOVER_UTIL_QUEUE_HPP