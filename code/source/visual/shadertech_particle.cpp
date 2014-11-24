#include "resources/cache.hpp"
#include "shader.hpp"
#include "shader_mgr.hpp"
#include "shadertech_particle.hpp"

namespace clover {
namespace visual {

void ParticleST::use()
{
	WorldShaderTech::use(getShaderMgr().getShader("visual_particle"));
}

} // visual
} // clover
