#include "mem_chunk.hpp"
#include "ensure.hpp"

namespace clover {
namespace util {

MemChunk::MemChunk(SizeType size)
		: memory(new uint8[size])
		, sizeBytes(size)
		, acquired(false){
}

MemChunk::~MemChunk(){
	ensure(!acquired);

	delete [] memory.get();
}

void MemChunk::acquire(){
	ensure(!acquired);
	ensure(memory);
	acquired= true;
}

void MemChunk::release(){
	ensure(acquired);
	acquired= false;
}

} // util
} // clover