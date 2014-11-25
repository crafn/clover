#ifndef CLOVER_VISUAL_SHADER_HPP
#define CLOVER_VISUAL_SHADER_HPP

#include "build.hpp"
#include "util/class_preproc.hpp"
#include "util/color.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"
#include "util/tidy.hpp"
#include "vertexattribute.hpp"

#include <GL/glew.h>
#include <string>

namespace clover {
namespace visual {

/// Represents a shader program with vertex/geometry/fragment shaders
class Shader {
public:
	Shader();
	DEFAULT_MOVE(Shader);
	DELETE_COPY(Shader);
	virtual ~Shader();

	void setSources(util::Str8 vertex, util::Str8 geometry="", util::Str8 fragment= "");

	/// Transform feedback
	void setOutputVaryings(util::DynArray<util::Str8> varyings);

	void setDefine(const char* def, util::Str8 value="");
	void setDefine(const char* def, int32 value);

	template <typename T>
	void compile(){ compile(T::getAttributes()); }
	void compile(const util::DynArray<VertexAttribute>& attribs);
	bool isCompiled(){ return compiled; }

	void use();

	/// No dynamically allocated `name`s please
	void setUniform(const char* name, const uint8&, int32 count=1);
	void setUniform(const char* name, const uint16&, int32 count=1);
	void setUniform(const char* name, const uint32&, int32 count=1);
	void setUniform(const char* name, const int32&,	int32 count=1);
	void setUniform(const char* name, const real32&, int32 count=1, int32 dim=1);
	void setUniform(const char* name, const real64&, int32 count= 1);
	void setUniform(const char* name, const util::Vec2d&, int32 count=1);
	void setUniform(const char* name, const util::Vec2f&, int32 count=1);
	void setUniform(const char* name, const util::Vec3f&, int32 count=1);
	void setUniform(const char* name, const util::Vec4f&, int32 count=1);
	void setUniform(const char* name, const util::Color&, int32 count=1);
	void setUniform(const char* name, const util::Mat33f&, int32 count=1);
	void setUniform(const char* name, const util::Mat44f&, int32 count=1);

	void setTexture(hardware::GlState::TexTarget target,
					const char* name,
					uint32 tex,
					int32 slot);

	void clear();

private:
	uint32 createShader(util::Str8 &source, uint32 type);
	void bindAttributes(const util::DynArray<VertexAttribute>& attribs);
	uint32 uniformLoc(const char* name);
	void printInfoLog(uint32 shader) const;
	void printLinkInfoLog() const;

	Tidy<uint32> vertShd;
	Tidy<uint32> fragShd;
	Tidy<uint32> geomShd;
	Tidy<uint32> program;

	util::HashMap<const char*, uint32> nameToUniform;
	util::Str8 vertSource, fragSource, geomSource;
	util::DynArray<util::Str8> outputVaryings;
	util::DynArray<std::pair<util::Str8, util::Str8>> defines;
	util::Str8 versionStr;
	bool compiled;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADER_HPP
