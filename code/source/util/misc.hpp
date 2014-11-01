#ifndef CLOVER_UTIL_MISC_HPP
#define CLOVER_UTIL_MISC_HPP

#include "build.hpp"

#include <cstdarg>
#include <string>

namespace clover {
namespace util {

/// @todo Move to debug namespace
std::string getFilenameFromPath(const char8* s);
std::string getReducedFunctionName(const char8* s);

} // util
} // clover

#endif // CLOVER_UTIL_MISC_HPP
