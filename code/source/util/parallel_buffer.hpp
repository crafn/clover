#ifndef CLOVER_UTIL_PARALLEL_BUFFER_HPP
#define CLOVER_UTIL_PARALLEL_BUFFER_HPP

#include "build.hpp"
#include "hardware/clstate.hpp"

namespace clover {
namespace visual {

/// @todo Shouldn't be in util >:(
class BaseVertexArrayObject;
class Framebuffer;
class Texture;

} // visual
namespace util {

class ParallelQueue;

/// @note	Destroying/resetting can cause blocking if using kernel is running
class ParallelBuffer {
public:

	ParallelBuffer();
	ParallelBuffer(ParallelBuffer&&);
	ParallelBuffer(const ParallelBuffer&)= delete;

	virtual ~ParallelBuffer();

	ParallelBuffer& operator=(ParallelBuffer&&);
	ParallelBuffer& operator=(const ParallelBuffer&)= delete;

	template <typename T>
	void create(hardware::ClState::BufferFlag flags, T& first, uint32 element_count=1);

	/// @note After aliasing, modifying state of GL object through GL API is UB!
	void alias(hardware::ClState::BufferFlag flags, const visual::BaseVertexArrayObject& vao);
	void alias(hardware::ClState::BufferFlag flags, const visual::Framebuffer& fbo);
	void alias(hardware::ClState::BufferFlag flags, const visual::Texture& tex);
	void aliasTex(hardware::ClState::BufferFlag flags, uint32 tex_target, uint32 tex_id);

	void attachToQueue(util::ParallelQueue& q){ attachedQueue= &q; }

	/// Reads data back to elements specified in create(..)
	void read();

	void acquire();
	void release();

	cl_mem getDId() const { return buffer.id; }

	void reset();

private:
	util::ParallelQueue* attachedQueue;
	hardware::ClState::Buffer buffer;
	void* hostData;
	SizeType hostDataSize;
};

template <typename T>
void ParallelBuffer::create(hardware::ClState::BufferFlag flags, T& first, uint32 element_count){
	ensure(hardware::gClState);
	hostData= &first;
	hostDataSize= sizeof(T)*element_count;

	ensure(buffer.id == 0);
	buffer=
		hardware::gClState->createBuffer<T>(
			hardware::gClState->getDefaultContext(),
			(hardware::ClState::BufferFlag)flags,
			first,
			element_count);
}

} // util
} // clover

#endif // CLOVER_UTIL_PARALLEL_BUFFER_HPP