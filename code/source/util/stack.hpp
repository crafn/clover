#ifndef CLOVER_UTIL_STACK_HPP
#define CLOVER_UTIL_STACK_HPP

#include "build.hpp"

#include <stack>

namespace clover {
namespace util {

template <typename T>
using Stack= std::stack<T>;

} // util
} // clover

#endif // CLOVER_UTIL_STACK_HPP