#include "shadertech_particle.hpp"
#include "resources/cache.hpp"
#include "shader.hpp"

namespace clover {
namespace visual {

ParticleST::ParticleST(){
	util::DynArray<Shader>& shds= resources::gCache->getShaders(resources::Shader_Particle);

	for (int32 i= 0; i < (int)shds.size(); ++i){
		shaders.pushBack(&shds[i]);
		locateUniforms(i);
	}
}

} // visual
} // clover