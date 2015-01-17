#ifndef CLOVER_UTIL_HASH_HPP
#define CLOVER_UTIL_HASH_HPP

#include "build.hpp"

#include <array>

namespace clover {
namespace util {

template <typename T> struct Hash32;

template <typename T>
uint32 hash32(const T& t){
	return Hash32<T>()(t);
}

/// Actual hashing function
ENGINE_API uint32 byteHash32(const uint8* buf, SizeType size);

/// To prevent copy-pasting in similar but not same array-like types
template <typename T>
uint32 rawArrayHash32(const T* buf, SizeType size){
	if (!buf)
		return 0;
	return byteHash32(reinterpret_cast<const uint8*>(buf), size*sizeof(T));
}

/// Hash for std::array
template <typename T, SizeType N>
struct Hash32<std::array<T, N>> {
	uint32 operator()(const std::array<T, N>& arr) const {
		return rawArrayHash32(arr.data(), arr.size());
	}
};

template <>
struct Hash32<real32> {
	uint32 operator()(real32 value) const;
};

template <>
struct Hash32<real64> {
	uint32 operator()(real64 value) const;
};

template <>
struct Hash32<bool> {
	uint32 operator()(bool value) const {
		return value ? 1 : 0;
	}
};

template <typename T>
struct Hash32<T*> {
	uint32 operator()(T* value) const {
		return static_cast<uint32>(reinterpret_cast<std::uintptr_t>(value)); \
	}
};

#define TRIVIAL_HASH(x) \
template<> \
struct Hash32<x> { \
	uint32 operator()(const x& value) const { \
		return static_cast<uint32>(value); \
	} \
};

TRIVIAL_HASH(int16)
TRIVIAL_HASH(int32)
TRIVIAL_HASH(char)
TRIVIAL_HASH(uint8)
TRIVIAL_HASH(uint16)
TRIVIAL_HASH(uint32)

#undef TRIVIAL_HASH

} // util
} // clover

#endif // CLOVER_UTIL_HASH_HPP
