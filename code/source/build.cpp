#include "build.hpp"

namespace clover {

#if ARCHITECTURE == ARCHITECTURE_32
#define BUILD_ARCHITECTURE_STR "32bit"
#else
#define BUILD_ARCHITECTURE_STR "64bit"
#endif

#if OS == OS_LINUX
#define BUILD_OS_STR "Linux"
#elif OS == OS_WINDOWS
#define BUILD_OS_STR "Windows"
#elif OS == OS_MACOSX
#define BUILD_OS_STR "MacOSX"
#endif

#if defined(DEBUG)
#define BUILD_MODE_STR "debug"
#elif defined(RELEASE)
#define BUILD_MODE_STR "release"
#else
#define BUILD_MODE_STR "development"
#endif

const char* getBuildStr(){
	return BUILD_ARCHITECTURE_STR " " BUILD_OS_STR " " BUILD_MODE_STR " build, " __DATE__ " " __TIME__;
}

#undef BUILD_ARCHITECTURE_STR
#undef BUILD_OS_STR
#undef BUILD_MODE_STR

} // clover
