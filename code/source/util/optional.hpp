#ifndef UTIL_OPTIONAL_HPP
#define UTIL_OPTIONAL_HPP

#include "build.hpp"

/// @todo Don't use boost
#include <boost/optional.hpp>

namespace clover {
namespace util {

template <typename T>
using Optional= boost::optional<T>;

const auto optionalNone= boost::none;

} // util
} // clover

#endif // UTIL_OPTIONAL_HPP
