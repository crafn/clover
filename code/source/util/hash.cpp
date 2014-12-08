#include "hash.hpp"
#include "profiling.hpp"

namespace clover {
namespace util {

uint32 byteHash32(const uint8* buf, SizeType size){
#define USE_DUFF_OPTIMZ 1
	PROFILE();

	// Modified FNV-1a
	uint32 hash= 2166136261;

#if USE_DUFF_OPTIMZ
#define HASH \
	do { hash= ((hash ^ *buf++) + 379721)*16777619; } while (0)
	if (size == 0)
		return hash;
	SizeType n= (size + 7)/8;
	switch (size % 8) {
		case 0: do { HASH;
		case 7: HASH;
		case 6: HASH;
		case 5: HASH;
		case 4: HASH;
		case 3: HASH;
		case 2: HASH;
		case 1: HASH; } while (--n > 0);
	}	
#undef HASH
#else
	for (SizeType i= 0; i < size; ++i){
		hash= ((hash ^ buf[i]) + 379721)*16777619;
	}
#endif

	return hash;
}

uint32 Hash32<real32>::operator()(real32 value) const {
	union { real32 f; uint32 i; } u;
	u.f= value;
	return u.i;
}

uint32 Hash32<real64>::operator()(real64 value) const {
	union { real64 f; uint32 i[2]; } u;
	u.f= value;
	return u.i[0]^u.i[1];
}

} // util
} // clover
