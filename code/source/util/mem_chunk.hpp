#ifndef CLOVER_UTIL_MEM_CHUNK_HPP
#define CLOVER_UTIL_MEM_CHUNK_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/tidy_ptr.hpp"

namespace clover {
namespace util {

/// Contiguous block of memory for memory pools
class MemChunk {
public:
	MemChunk(SizeType size, const char* tag);
	DELETE_COPY(MemChunk);
	DEFAULT_MOVE(MemChunk);
	~MemChunk();

	void acquire();
	void release();

	uint8* begin() const { return memory.get(); }
	uint8* end() const { return memory.get() + size(); }
	SizeType size() const { return sizeBytes; }

	const char* getTag() const { return tag; }

private:
	TidyPtr<uint8> memory;
	SizeType sizeBytes;
	bool acquired;
	const char* tag;
};

} // util
} // clover

#endif // CLOVER_UTIL_MEM_CHUNK_HPP
