#ifndef CLOVER_UTIL_DEQUE_HPP
#define CLOVER_UTIL_DEQUE_HPP

#include "build.hpp"

#include <deque>

namespace clover {
namespace util {

template <typename T>
using Deque = std::deque<T>;

} // util
} // clover

#endif // CLOVER_UTIL_DEQUE_HPP