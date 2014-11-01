#ifndef CLOVER_UTIL_BIT_HPP
#define CLOVER_UTIL_BIT_HPP

#include "build.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace util {

constexpr SizeType bitsInByte= 8;

template <typename T>
constexpr SizeType bitSize(){ return sizeof(T)*bitsInByte; }

/// @param shift Positive for right-shifting
template <typename T>
T bitRotated(T t, int32 shift){
	shift= shift % bitSize<T>();
	if (shift > 0)
		return (t >> shift) | (t << (bitSize<T>() - shift));
	else
		return (t << -shift) | (t >> (bitSize<T>() + shift));
}

} // util
} // clover

#endif // CLOVER_UTIL_BIT_HPP
