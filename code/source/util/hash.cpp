#include "hash.hpp"
#include "profiling.hpp"

namespace clover {
namespace util {

uint32 byteHash32(const uint8* buf, SizeType size){
	// Modified FNV-1a
	uint32 hash= 2166136261;
	for (SizeType i= 0; i < size; ++i){
		hash= ( (hash ^ hash32(buf[i])) + 987654321 )*16777619;
		// Rotation seems to prevent some collisions with arrays in which
		// single bits are swapped between array elements.
		//hash= bitRotatedLeft<uint32>(hash, hash % 31);
		// Simpler rotation - rendering time improved by 20%
		hash= (hash << 9) | (hash >> 21);
	}
	return hash;
}

uint32 Hash32<real32>::operator()(real32 value) const {
	union { real32 f; uint32 i; } u;
	u.f= value;
	return hash32(u.i);
}

uint32 Hash32<real64>::operator()(real64 value) const {
	union { real64 f; uint32 i[2]; } u;
	u.f= value;
	return u.i[0]^u.i[1];
}

} // util
} // clover
