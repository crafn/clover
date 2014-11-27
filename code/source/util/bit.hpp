#ifndef CLOVER_UTIL_BIT_HPP
#define CLOVER_UTIL_BIT_HPP

#include "build.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace util {

constexpr SizeType bitsInByte= 8;

template <typename T>
constexpr SizeType bitSize() { return sizeof(T)*bitsInByte; }

template <typename T>
T bitRotatedLeft(T t, uint32 shift)
{
	shift= shift % bitSize<T>();
	return (t << shift) | (t >> (bitSize<T>() - shift));
}

template <typename T>
T bitRotatedRight(T t, uint32 shift)
{
	shift= shift % bitSize<T>();
	return (t >> shift) | (t << (bitSize<T>() - shift));
}

/// @param shift Positive for right-shifting
template <typename T>
T bitRotated(T t, int32 shift)
{
	shift= shift % bitSize<T>();
	if (shift > 0)
		return bitRotatedRight(t, static_cast<uint32>(shift));
	else
		return bitRotatedLeft(t, static_cast<uint32>(-shift));
}

} // util
} // clover

#endif // CLOVER_UTIL_BIT_HPP
