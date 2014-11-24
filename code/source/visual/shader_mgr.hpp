#ifndef CLOVER_VISUAL_SHADER_MGR_HPP
#define CLOVER_VISUAL_SHADER_MGR_HPP

#include "build.hpp"
#include "shader.hpp"
#include "util/hashmap.hpp"

namespace clover {
namespace visual {

class ShaderOptions;

/// Manages individual shaders
class ShaderMgr {
public:
	using ShaderId= uint32;

	/// @note Can perform shader compilation
	/// @warning Don't keep pointer to return value
	Shader& getShader(const util::Str8& name);
	Shader& getShader(const util::Str8& name, const ShaderOptions& options);

	SizeType getShaderCount() const { return shaders.size(); }

private:
	util::HashMap<ShaderId, Shader> shaders;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADER_MGR_HPP
