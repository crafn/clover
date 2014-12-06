#include "parallel_buffer.hpp"
#include "util/parallel_queue.hpp"
#include "util/profiling.hpp"
#include "visual/framebuffer.hpp"
#include "visual/texture.hpp"
#include "visual/vertexarrayobject.hpp"

namespace clover {
namespace util {

ParallelBuffer::ParallelBuffer()
		: attachedQueue(nullptr)
		, hostData(nullptr)
		, hostDataSize(0){
	buffer.id= 0;
}

ParallelBuffer::ParallelBuffer(ParallelBuffer&& other){
	operator=(std::move(other));
}

ParallelBuffer::~ParallelBuffer(){
	reset();
}

ParallelBuffer& ParallelBuffer::operator=(ParallelBuffer&& other){
	reset();

	attachedQueue= other.attachedQueue;
	buffer= std::move(other.buffer);
	other.buffer.id= 0;
	hostData= other.hostData;

	return *this;
}

void ParallelBuffer::alias(hardware::ClState::BufferFlag flags, const visual::BaseVertexArrayObject& vao){
	ensure(hardware::gClState);
	ensure(buffer.id == 0);
	buffer=
		hardware::gClState->createFromGLBuffer(
				hardware::gClState->getDefaultContext(),
				flags,
				vao.getVboDId());
}

void ParallelBuffer::alias(hardware::ClState::BufferFlag flags, const visual::Framebuffer& fbo){
	ensure(buffer.id == 0);
	aliasTex(flags, (uint32)fbo.getTexTarget(), fbo.getTextureDId());
}

void ParallelBuffer::alias(hardware::ClState::BufferFlag flags, const visual::Texture& tex){
	ensure(tex.getDimensions() != util::Vec2i(0));
	aliasTex(flags, GL_TEXTURE_2D, tex.getDId());
}

void ParallelBuffer::aliasTex(hardware::ClState::BufferFlag flags, uint32 tex_target, uint32 tex_id){
	ensure(hardware::gClState);
	ensure(buffer.id == 0);
	buffer=
		hardware::gClState->createFromGLTexture(
				hardware::gClState->getDefaultContext(),
				flags,
				tex_target,
				tex_id);
}

void ParallelBuffer::read(){
	ensure(hardware::gClState);
	ensure(hostData);
	ensure(attachedQueue);

	bool blocking= true;
	SizeType offset= 0;
	hardware::gClState->enqueueReadBuffer(
			attachedQueue->queue,
			buffer,
			blocking,
			offset,
			hostDataSize,
			hostData);
}

void ParallelBuffer::acquire(){
	ensure(hardware::gClState);
	ensure(attachedQueue);
	hardware::gClState->acquireBuffer(attachedQueue->queue, buffer);
}

void ParallelBuffer::release(){
	ensure(hardware::gClState);
	ensure(attachedQueue);
	hardware::gClState->releaseBuffer(attachedQueue->queue, buffer);
}

void ParallelBuffer::reset(){
	{ PROFILE();
		if (buffer.id){
			hardware::gClState->destroyBuffer(buffer);
		}
	}
	buffer.id= 0;
}

} // util
} // clover