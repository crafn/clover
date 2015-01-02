#ifndef CLOVER_BUILD_HPP
#define CLOVER_BUILD_HPP

#define PLATFORM_UNKNOWN 0
#define PLATFORM_UNIX 1
#define PLATFORM_WINDOWS 2

#define OS_UNKNOWN 0
#define OS_LINUX 1
#define OS_WINDOWS 2
#define OS_MACOSX 3

#define ARCHITECTURE_32 1
#define ARCHITECTURE_64 2

#define THREADS_PTHREADS 1
#define THREADS_WIN32 2

#if defined(__linux)
	#define OS OS_LINUX
	#define PLATFORM PLATFORM_UNIX
	#define THREADS THREADS_PTHREADS
#elif defined(__WIN32)
	#define OS OS_WINDOWS
	#define PLATFORM PLATFORM_WINDOWS
	#define THREADS THREADS_PTHREADS
#elif defined (__APPLE__) || defined(MACOSX)
	#define OS OS_MACOSX
	#define PLATFORM PLATFORM_UNIX
	#define THRAEDS THREADS_PTHREADS
#else
	#define OS OS_UNKNOWN
	#define PLATFORM PLATFORM_UNKNOWN
	#define THRAEDS THRAEDS_PTHREADS
#endif

#if defined(__LP64__) || defined(_WIN64)
#define ARCHITECTURE ARCHITECTURE_64
#else
#define ARCHITECTURE ARCHITECTURE_32
#endif

/// @todo Determine correctly
#define ATOMIC_PTR_READWRITE true

#include <cstddef>
#include <cstdint>

namespace clover {
namespace util { class Str8; }

const char* getBuildStr();

// Clover can be built in 3 modes: debug, development and release
//	 debug: controlled by DEBUG, uses all possible debug stuff, including debug_ensure
//	 dev: ensure + release_ensure
//	 release: controlled by RELEASE, uses release_ensure

/// @todo E.g. #define DEBUG -> #define BUILD_MODE BUILD_MODE_DEBUG

typedef float real32;
typedef double real64;

typedef int8_t int8;
typedef uint8_t uint8;

typedef int16_t int16;
typedef uint16_t uint16;

typedef int32_t int32;
typedef uint32_t uint32;

typedef int64_t int64;
typedef uint64_t uint64;

typedef char char8;

typedef std::size_t SizeType;

} // clover

#if defined(DEBUG) && defined(RELEASE)
	#error DEBUG and RELEASE both defined
#endif

#endif
