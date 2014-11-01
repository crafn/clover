#ifndef CLOVER_VISUAL_VERTEX_ARRAY_OBJECT_HPP
#define CLOVER_VISUAL_VERTEX_ARRAY_OBJECT_HPP

#include "build.hpp"
#include "hardware/glstate.hpp"
#include "util/arrayview.hpp"
#include "util/string.hpp"
#include "vertexattribute.hpp"

namespace clover {
namespace visual {

/// A wrapper for OpenGL vertex array object
class BaseVertexArrayObject {
public:
	BaseVertexArrayObject(hardware::GlState::Primitive p, uint32 size_of_one_vertex, hardware::GlState::Type index_type= hardware::GlState::Type::None);
	BaseVertexArrayObject(const BaseVertexArrayObject&)= delete;
	BaseVertexArrayObject(BaseVertexArrayObject&&);
	virtual ~BaseVertexArrayObject();

	void setUsage(hardware::GlState::VaoUsage u){ usage= u; }

	void draw() const;
	void drawRange(uint32 start, uint32 end) const;

	bool isIndexed() const { return indexed; }
	hardware::GlState::VaoDId getVaoDId() const { return vaoDId; }
	hardware::GlState::VboDId getVboDId() const { return vboDId; }

	uint32 getVertexCount() const { return vertexCount; }
	uint32 getIndexCount() const { return indexCount; }

	void setPrimitive(hardware::GlState::Primitive p){ primitive= p; }
	hardware::GlState::Primitive getPrimitive() const { return primitive; }

protected:
	/// Tells how to separate things in the vertex data
	/// Needs to be called only once for each attribute
	void setVertexAttribute(uint32 i, uint32 offset_in_bytes, hardware::GlState::Type type, uint32 count, bool normalized=false, bool enable=true);

	/// Uploads vertex/index data to the GPU
	/// @param data If null, only allocating memory
	void submitData(hardware::GlState::VaoBufferType type, const void* data, uint32 count);

	void overwriteData(hardware::GlState::VaoBufferType type, uint32 offset, const void* data, uint32 count);

	void bind() const;
	void unbind() const;

private:
	void genBuffers();
	void deleteBuffers();

	bool indexed;

	hardware::GlState::VaoDId vaoDId; /// Internal vertex array object id
	hardware::GlState::VboDId vboDId; /// Internal vertex buffer object id
	hardware::GlState::IboDId iboDId; /// Internal index buffer object id

	hardware::GlState::Type indexType;

	uint32 vertexCount;
	uint32 indexCount;

	uint32 indexSize;
	uint32 vertexSize;

	util::DynArray<uint32> enabledAttribArrays;

	hardware::GlState::VaoUsage usage;
	hardware::GlState::Primitive primitive;
};


struct IndexTypeNone {
	typedef int noIndex;
	IndexTypeNone& operator+=(int32 i){ return *this; }
	IndexTypeNone& operator=(int32 i){ return *this; }
};

template <typename T>
struct IsIndexTypeNoIndex {
	typedef char yes[1];
	typedef char no[2];

	template <typename U>
	static yes& is(typename U::noIndex*);

	template <typename U>
	static no& is(...);

	static const bool value= sizeof(is<T>(0)) == sizeof(yes);
};

///	VertexArrayObject A template class for automatic handling of vertex size & offset thingies in vertex array object
/// Vertex type V needs a static method: util::DynArray<visual::VertexAttribute> getResourceAttributes()
template <typename V, typename I=IndexTypeNone>
class VertexArrayObject : public BaseVertexArrayObject {
public:

	VertexArrayObject(hardware::GlState::Primitive p= hardware::GlState::Primitive::Triangle);
	virtual ~VertexArrayObject();

	/// @brief Allocates and uploads data to GPU
	void submit(util::ArrayView<const V> vertices);

	/// @brief Allocates space in GPU
	void reserveVertices(uint32 count);

	/// @brief Submits data to already allocated space
	void overwrite(util::ArrayView<const V> vertices, uint32 offset= 0);

	// Use only if indexed
	void submit(util::ArrayView<const I> indices);
	void reserveIndices(uint32 count);
	void overwrite(util::ArrayView<const I> indices, uint32 offset= 0);

private:
};

#include "vertexarrayobject.tpp"

} // visual
} // clover

#endif // CLOVER_VISUAL_VERTEX_ARRAY_OBJECT_HPP