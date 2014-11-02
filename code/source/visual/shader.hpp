#ifndef CLOVER_VISUAL_SHADER_HPP
#define CLOVER_VISUAL_SHADER_HPP

#include "build.hpp"
#include "util/color.hpp"
#include "util/string.hpp"
#include "vertexattribute.hpp"

#include <GL/glew.h>
#include <string>

namespace clover {
namespace visual {

/// Represents a shader program with vertex/geometry/fragment shaders
class Shader {
public:
	Shader();
	virtual ~Shader();

	void setSources(util::Str8 vertex, util::Str8 geometry="", util::Str8 fragment= "");

	/// Transform feedback
	void setOutputVaryings(util::DynArray<util::Str8> varyings);

	void setDefine(util::Str8 def, util::Str8 value="");
	void setDefine(util::Str8 def, int32 value);

	template <typename T>
	void compile(){ compile(T::getAttributes()); }
	void compile(const util::DynArray<VertexAttribute>& attribs);
	bool isCompiled(){ return compiled; }

	void use();

	GLint getUniformLocation(const char* str);

	void setUniform(uint32 loc, const uint8&, int32 count=1);
	void setUniform(uint32 loc, const uint16&, int32 count=1);
	void setUniform(uint32 loc, const uint32&, int32 count=1);
	void setUniform(uint32 loc, const int32&,	int32 count=1);
	void setUniform(uint32 loc, const real32&, int32 count=1, int32 dim=1);
	void setUniform(uint32 loc, const real64&, int32 count= 1);
	void setUniform(uint32 loc, const util::Vec2d&, int32 count=1);
	void setUniform(uint32 loc, const util::Vec2f&, int32 count=1);
	void setUniform(uint32 loc, const util::Vec3f&, int32 count=1);
	void setUniform(uint32 loc, const util::Vec4f&, int32 count=1);
	void setUniform(uint32 loc, const util::Color&, int32 count=1);
	void setUniform(uint32 loc, const util::Mat33f&, int32 count=1);
	
	/// Deprecated
	void setTexture(uint32 loc, uint32 tex, int32 slot);

	void setTexture(hardware::GlState::TexTarget target,
					uint32 loc,
					uint32 tex,
					int32 slot);

	void clear();

private:
	uint32 createShader(util::Str8 &source, uint32 type);
	void bindAttributes(const util::DynArray<VertexAttribute>& attribs);
	void printInfoLog(uint32 shader) const;
	void printLinkInfoLog() const;

	uint32 vertShd;
	uint32 fragShd;
	uint32 geomShd;
	
	uint32 program;

	util::Str8 vertSource, fragSource, geomSource;
	util::DynArray<util::Str8> outputVaryings;

	util::DynArray<std::pair<util::Str8, util::Str8>> defines;

	util::Str8 versionStr;
	bool compiled;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADER_HPP