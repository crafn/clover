#include "shader_mgr.hpp"

namespace clover {
namespace visual {

Shader& ShaderMgr::getShader(const util::Str8&, const ShaderOptions& options)
{
	release_ensure(0 && "@todo");
	return shaders[0];
}

} // visual
} // clover
