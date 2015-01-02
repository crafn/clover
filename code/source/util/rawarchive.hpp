#ifndef CLOVER_UTIL_RAWARCHIVE_HPP
#define CLOVER_UTIL_RAWARCHIVE_HPP

#include "build.hpp"
#include "serialization.hpp"

#include <cstring>

namespace clover {
namespace util {

/// Binary archive which contains minimum data of serialized objects
/// i.e. no field names
/// @todo Worry about endianness and floats
/// @todo Implementation
class RawArchive {
public:

	/// @return Serialized byte count
	template <typename T>
	static SizeType serializePod(uint8* buf, const T* ptr, SizeType count= 1);

	/// @return Deserialized byte count
	template <typename T>
	static SizeType deserializePod(const uint8* buf, T* ptr, SizeType count= 1);

private:
};

#include "rawarchive.tpp"

} // util
} // clover

#endif // CLOVER_UTIL_RAWARCHIVE_HPP
