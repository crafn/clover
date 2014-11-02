#ifndef CLOVER_BUILD_HPP
#define CLOVER_BUILD_HPP

/// @todo Prefix for macros

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

#if __LP64__
#define ARCHITECTURE ARCHITECTURE_64
#else
#define ARCHITECTURE ARCHITECTURE_32
#endif

/// @todo Determine correctly
#define ATOMIC_PTR_READWRITE true

#include <cstdint>
#include <cstring>

namespace clover {

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

typedef char	   char8;

typedef size_t SizeType;

/// @todo Move these somewhere else
// UTF-8 string, u8"String literal in C++11"
namespace util {
class Str8;

template<typename T>
class Angle;

typedef Angle<real32> Angle1f;
typedef Angle<real64> Angle1d;
typedef Angle1f Rotation;


template <typename T, SizeType N>
class RealVector;

template <typename T, SizeType N>
class IntegerVector;

typedef RealVector<real32, 2> Vec2f;
typedef RealVector<real32, 3> Vec3f;
typedef RealVector<real32, 4> Vec4f;

typedef RealVector<real64, 2> Vec2d;
typedef RealVector<real64, 3> Vec3d;
typedef RealVector<real64, 4> Vec4d;

typedef IntegerVector<int32, 2> Vec2i;
typedef IntegerVector<int32, 3> Vec3i;

typedef IntegerVector<int64, 2> Vec2l;
typedef IntegerVector<int64, 3> Vec3l;

template <typename T>
class Quaternion;

typedef Quaternion<real32> Quatf;
typedef Quaternion<real64> Quatd;

template <typename P, typename R>
class RtTransform;

template <typename P, typename R, typename S>
class SrtTransform;

typedef RtTransform<real32, Vec2f> RtTransform2f;
typedef SrtTransform<Vec2f, real32, Vec2f> SrtTransform2f;
typedef SrtTransform<Vec3f, Quatf, Vec3f> SrtTransform3f;

typedef RtTransform<real64, Vec2d> RtTransform2d;
typedef RtTransform<Quatd, Vec3d> RtTransform3d;
typedef SrtTransform<Vec2d, real64, Vec2d> SrtTransform2d;
typedef SrtTransform<Vec3d, Quatd, Vec3d> SrtTransform3d;

template<typename T, SizeType N>
class Matrix;

typedef Matrix<real32, 3> Mat33f;
typedef Matrix<real32, 4> Mat44f;

typedef Matrix<real64, 3> Mat33d;
typedef Matrix<real64, 4> Mat44d;

} // util

typedef util::Vec2d WorldVec;
typedef util::Vec2i BlockVec;
typedef util::Vec2i ChunkVec;
typedef util::Vec2i RegionVec;

} // clover

#if defined(DEBUG) && defined(RELEASE)
	#error DEBUG and RELEASE both defined
#endif

#endif