#ifndef CLOVER_HARDWARE_GLSTATE_HPP
#define CLOVER_HARDWARE_GLSTATE_HPP

#include "build.hpp"
#include "util/color.hpp"
#include "util/map.hpp"
#include "util/math.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define BUFFER_OFFSET(i) ((int8 *)NULL + (i))

namespace clover {
namespace hardware {

/// OpenGL context wrapper
class GlState {
public:

	enum class VaoUsage {
		StaticDraw= GL_STATIC_DRAW,
		StaticRead= GL_STATIC_READ,
		StaticCopy= GL_STATIC_COPY,
		DynamicDraw= GL_DYNAMIC_DRAW,
		DynamicRead= GL_DYNAMIC_READ,
		DynamicCopy= GL_DYNAMIC_COPY,
		StreamDraw= GL_STREAM_DRAW,
		StreamRead= GL_STREAM_READ,
		StreamCopy= GL_STREAM_COPY
	};

	enum class Type {
		None= 0,
		Int8= GL_BYTE,
		Uint8= GL_UNSIGNED_BYTE,
		Int16= GL_SHORT,
		Uint16= GL_UNSIGNED_SHORT,
		Int32= GL_INT,
		Uint32= GL_UNSIGNED_INT,
		Real32= GL_FLOAT
	};

	enum class Primitive {
		Triangle= GL_TRIANGLES,
		Point= GL_POINTS
	};

	enum class TexTarget {
		Tex2d= GL_TEXTURE_2D,
		Tex2dArray= GL_TEXTURE_2D_ARRAY,
		Tex3d= GL_TEXTURE_3D
	};

	enum class TexFormat {
		Rgb8= GL_RGB8,
		Rgba8= GL_RGBA8,
		Rgb16= GL_RGB16,
		Rgba16= GL_RGBA16,
		Depth= GL_DEPTH_COMPONENT
	};

	template <typename T>
	struct TypeToEnumType {
		static GlState::Type value(){ return GlState::Type::None; }
	};


	enum class VaoBufferType {
		Vertex= GL_ARRAY_BUFFER,
		Index= GL_ELEMENT_ARRAY_BUFFER
	};

	static uint32 getSize(Type t){
		switch(t){
			case Type::None: return 0;
			case Type::Int8: return sizeof(int8);
			case Type::Uint8: return sizeof(uint8);
			case Type::Int16: return sizeof(int16);
			case Type::Uint16: return sizeof(uint16);
			case Type::Int32: return sizeof(int32);
			case Type::Uint32: return sizeof(uint32);
			case Type::Real32: return sizeof(real32);
			default: release_ensure_msg(0, "Not implemented");
		}
	}
	
	static SizeType getMaxValue(Type t){
		switch(t){
			case Type::None: return 0;
			case Type::Int8: return std::numeric_limits<int8>::max();
			case Type::Uint8: return std::numeric_limits<uint8>::max();
			case Type::Int16: return std::numeric_limits<int16>::max();
			case Type::Uint16: return std::numeric_limits<uint16>::max();
			case Type::Int32: return std::numeric_limits<int32>::max();
			case Type::Uint32: return std::numeric_limits<uint32>::max();
			default: release_ensure_msg(0, "Not implemented");
		}
	}

	using BlendFactor= GLenum;

	struct BlendFunc {
		BlendFactor srcFactor;
		BlendFactor dstFactor;
	};

	using TexDId= GLuint;
	using FboDId= GLuint; /// Frame buffer object
	using VaoDId= GLuint; /// Vertex array object
	using VboDId= GLuint; /// Vertex buffer object
	using IboDId= GLuint; /// Index buffer object
	using ProgramDid= GLuint; /// Shader program

    GlState();

    TexDId genTex();

	/// If possible, use bindTex(tex, unit) instead of separately setting texture unit,
	/// because bindTex prevents unnecessary state changes
    void setTextureUnit(int32 unit);

	/// Binding overrides previous bind in the same unit,
	/// even if target is not the same
    void bindTex(TexTarget target, TexDId tex, int32 unit= -1);

    void setPolygonMode(GLenum mode);
    void setBlendFunc(BlendFunc func);

    void setViewport(util::Vec2i pos, util::Vec2i size);

	template <typename T>
    void setTexParam(GLenum p, const T& v){ ensure_msg(0, "Invalid template type"); }
    void setDefaultTexParams();
	

    void submitTexData(	TexFormat format,
						Type datatype,
						util::Vec2i size,
						const void* data,
						int32 miplevel= 0);

	void submitTexData(	TexFormat format,
						Type datatype,
						util::Vec3i size_3d,
						const void* data,
						int miplevel= 0);

    void generateMipmap();
    void deleteTex(uint32 tex);

	FboDId genFbo();
    void bindFbo(FboDId fbo);
	void attachTexToFbo(GLenum attachment, TexDId tex, int32 miplevel= 0); 
	GLenum getFboStatus();
	void deleteFbo(FboDId fbo);

    void setClearColor(const util::Color& c);
    void clear(GLbitfield);

	void errorCheck(const util::Str8& where);

	void genVertexArrays(uint32 count, VaoDId* vao);
	void genVertexBuffers(uint32 count, VboDId* vbo);
	void genIndexBuffers(uint32 count, IboDId* ibo);

	void bindVertexArray(VaoDId id);
	void bindVertexBuffer(VboDId id);
	void bindIndexBuffer(IboDId id);

	void vertexAttribPointer(uint32 i, uint32 count, Type type, bool normalized, uint32 vertexSize, uint32 offset_in_bytes);
	void enableVertexAttribArray(uint32 i, bool enable=true);

	void submitBufferData(VaoBufferType type, uint32 byte_count, const void* data, VaoUsage usage);
	void overwriteBufferData(VaoBufferType type, uint32 byte_offset, uint32 byte_count, const void* data);

	/// @brief Indexed draw
	void drawElements(Primitive p, uint32 draw_index_offset, uint32 draw_index_count, Type index_type, uint32 max_vertex_index);

	/// @brief Non-indexed draw
	void drawArrays(Primitive p, uint32 draw_index_offset, uint32 draw_index_count);

	void deleteVertexArrays(uint32 count, VaoDId* id);
	void deleteVertexBuffers(uint32 count, VboDId* id);
	void deleteIndexBuffers(uint32 count, IboDId* id);
	
	void useProgram(ProgramDid p);

	void flush();
	void finish();

	/// Translates string to constant
	/// e.g. asConstant("GL_ZERO") == GL_ZERO
	uint32 asConstant(const util::Str8& constant_str, uint32 error_value= 0) const;

private:
	GLenum activeGlTexTarget() const;
	GLenum defaultGlLayout(TexFormat format) const;

    /// @todo To non-hard-coded
    static const int32 texUnitCount=32;

    int32 activeTexUnit;
    TexDId boundTex[texUnitCount];
	TexTarget boundTexTargets[texUnitCount];

    GLenum polygonMode;

    GLenum blendSource, blendDestination;

    FboDId boundFbo;
	VaoDId boundVao;
	VboDId boundVbo;
	IboDId boundIbo;

	ProgramDid activeProgram;

    util::Vec2i viewportPosition;
    util::Vec2i viewportSize;

};

template <>
void GlState::setTexParam<int32>(GLenum e, const int32& v);
template <>
void GlState::setTexParam<real32>(GLenum e, const real32& v);
template <>
void GlState::setTexParam<real64>(GLenum e, const real64& v);

template <>
struct GlState::TypeToEnumType<real32>{ static GlState::Type value(){ return GlState::Type::Real32; } };

template <>
struct GlState::TypeToEnumType<uint16>{ static GlState::Type value(){ return GlState::Type::Uint16; } };

template <>
struct GlState::TypeToEnumType<uint32>{ static GlState::Type value(){ return GlState::Type::Uint32; } };

extern GlState *gGlState;

} // hardware
} // clover

#endif // CLOVER_HARDWARE_GLSTATE_HPP