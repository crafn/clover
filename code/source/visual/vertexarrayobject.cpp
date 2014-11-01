#include "vertexarrayobject.hpp"

namespace clover {
namespace visual {

BaseVertexArrayObject::BaseVertexArrayObject(hardware::GlState::Primitive p, uint32 size_of_one_vertex, hardware::GlState::Type type):
		indexed(type != hardware::GlState::Type::None),
		vaoDId(0),
		vboDId(0),
		iboDId(0),
		indexType(type),
		vertexCount(0),
		indexCount(0),
		indexSize(hardware::GlState::getSize(type)),
		vertexSize(size_of_one_vertex),
		usage(hardware::GlState::VaoUsage::StaticDraw),
		primitive(p){
	genBuffers();
}
BaseVertexArrayObject::BaseVertexArrayObject(BaseVertexArrayObject&& other):
		indexed(other.indexed),
		vaoDId(other.vaoDId),
		vboDId(other.vboDId),
		iboDId(other.iboDId),
		indexType(other.indexType),
		vertexCount(other.vertexCount),
		indexCount(other.indexCount),
		indexSize(other.indexSize),
		vertexSize(other.vertexSize),
		usage(other.usage),
		primitive(other.primitive){
	other.vaoDId= 0;
	other.vboDId= 0;
	other.iboDId= 0;
	other.indexSize= 0;
	other.vertexSize= 0;
}
BaseVertexArrayObject::~BaseVertexArrayObject(){
	deleteBuffers();
}

void BaseVertexArrayObject::setVertexAttribute(uint32 i, uint32 offset_in_bytes, hardware::GlState::Type type, uint32 count, bool normalized, bool enable){
	bind();
	hardware::gGlState->vertexAttribPointer(i, count, type, normalized, vertexSize, offset_in_bytes);
	hardware::gGlState->enableVertexAttribArray(i, enable);

	if (enable)
		enabledAttribArrays.pushBack(i);
}

void BaseVertexArrayObject::submitData(hardware::GlState::VaoBufferType type, const void* data, uint32 count){
	ensure_msg(	type == hardware::GlState::VaoBufferType::Vertex || count <= hardware::GlState::getMaxValue(indexType),
				"Vertex count over index range: %i, %i", count, hardware::GlState::getMaxValue(indexType));
	
	bind();
	
	if (type == hardware::GlState::VaoBufferType::Vertex){
		vertexCount= count;
		hardware::gGlState->submitBufferData(hardware::GlState::VaoBufferType::Vertex, vertexSize*vertexCount, data, usage);
	}
	else if (type == hardware::GlState::VaoBufferType::Index){
		ensure(indexed);
		indexCount= count;
		hardware::gGlState->submitBufferData(hardware::GlState::VaoBufferType::Index, indexSize*indexCount, data, usage);
	}
}

void BaseVertexArrayObject::overwriteData(hardware::GlState::VaoBufferType type, uint32 offset, const void* data, uint32 count){
	if (count == 0) return;
	bind();

	if (type == hardware::GlState::VaoBufferType::Vertex){
		ensure(offset + count <= vertexCount);
		hardware::gGlState->overwriteBufferData(hardware::GlState::VaoBufferType::Vertex, vertexSize*offset, vertexSize*count, data);
	}
	else if (type == hardware::GlState::VaoBufferType::Index){
		ensure(indexed);
		ensure(offset + count <= indexCount);
		hardware::gGlState->overwriteBufferData(hardware::GlState::VaoBufferType::Index, indexSize*offset, indexSize*count, data);
	}
}

void BaseVertexArrayObject::draw() const {
	bind();

	if (indexed)
		hardware::gGlState->drawElements(primitive, 0, indexCount, indexType, vertexCount);
	else
		hardware::gGlState->drawArrays(primitive, 0, vertexCount);
}

void BaseVertexArrayObject::drawRange(uint32 start, uint32 end) const {
	if (end == start)
		return;
	
	bind();

	ensure(end > start);
	if (indexed)
		hardware::gGlState->drawElements(primitive, start, end-start, indexType, vertexCount);
	else
		hardware::gGlState->drawArrays(primitive, start, end-start);
}

void BaseVertexArrayObject::bind() const {
	ensure(vaoDId);
	hardware::gGlState->bindVertexArray(vaoDId);
	hardware::gGlState->bindVertexBuffer(vboDId);
	if (indexed)
		hardware::gGlState->bindIndexBuffer(iboDId);

}

void BaseVertexArrayObject::unbind() const {
	hardware::gGlState->bindVertexArray(0);
	hardware::gGlState->bindVertexBuffer(0);
	hardware::gGlState->bindIndexBuffer(0);
}

void BaseVertexArrayObject::genBuffers(){
	ensure(!vaoDId && !vboDId && !iboDId);

	hardware::gGlState->genVertexArrays(1, &vaoDId);
	hardware::gGlState->genVertexBuffers(1, &vboDId);
	if (indexed) hardware::gGlState->genIndexBuffers(1, &iboDId);

	ensure(vaoDId && vboDId && (iboDId || !indexed));
}

void BaseVertexArrayObject::deleteBuffers(){
	ensure(vaoDId && vboDId && (iboDId || !indexed));

	bind();

	for (auto  m : enabledAttribArrays){
		// Disable all vertex attributes
		hardware::gGlState->enableVertexAttribArray(m, false);
	}

	enabledAttribArrays.clear();

	unbind();

	hardware::gGlState->deleteVertexArrays(1, &vaoDId);
	hardware::gGlState->deleteVertexBuffers(1, &vboDId);
	if (indexed)
		hardware::gGlState->deleteIndexBuffers(1, &iboDId);
	vaoDId= 0;
	vboDId= 0;
	iboDId= 0;
}

} // visual
} // clover