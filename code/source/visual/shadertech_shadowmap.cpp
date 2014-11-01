#include "shadertech_shadowmap.hpp"
#include "framebuffer.hpp"
#include "global/cfg_mgr.hpp"
#include "hardware/glstate.hpp"
#include "mesh.hpp"
#include "resources/cache.hpp"
#include "shader.hpp"
// Debug
#include "hardware/keyboard.hpp"

namespace clover {
namespace visual {

ShadowMapST::ShadowMapST(){
	util::DynArray<Shader>& shd= resources::gCache->getShaders(resources::Shader_ShadowMap);
	for (int32 i=0; i<(int)shd.size(); i++){
		shaders.pushBack(&shd[i]);
		locateUniforms(i);
	}

	int32 size= global::gCfgMgr->get<int32>("visual::defaultShadowMapSize", 512);

	Framebuffer::Cfg fbo_cfg;
	fbo_cfg.resolution= util::Vec2i(size);
	fbo_cfg.linearInterpolation= true;
	temp.create(fbo_cfg);

	int32 w= size;
	while(w > 2){
		w /= 2;
		reduced.pushBack(Framebuffer());
		fbo_cfg.resolution= util::Vec2i{w, size};
		fbo_cfg.linearInterpolation= false;
		reduced.back().create(fbo_cfg);
	}

	TriMesh m;
	m.addRectByCenter(util::Vec2f(0.0), util::Vec2f(1.0), util::Vec2f(0.5), util::Vec2f(0.5));

	mapQuad.add(m);
	mapQuad.flush();
}

ShadowMapST::~ShadowMapST(){
	temp.destroy();

	for (int32 i=0; i<(int)reduced.size(); i++)
		reduced[i].destroy();
}

void ShadowMapST::locateUniforms(uint32 shader_i){
	if (shader_i >= texLoc.size()){
		texLoc.resize(shader_i+1);
		reductionLoc.resize(shader_i+1);
	}

	texLoc[shader_i]= shaders[shader_i]->getUniformLocation("uSampler");
	reductionLoc[shader_i]= shaders[shader_i]->getUniformLocation("uReductionSize");
}

void ShadowMapST::generate(hardware::GlState::TexDId castermap, Framebuffer& result){
	shaderIndex= Phase_Distort;
	temp.bind();

	use();
	shaders[shaderIndex]->setTexture(texLoc[shaderIndex], castermap, 0);

	mapQuad.draw();

	shaderIndex= Phase_Reduction2;
	use();

	uint32 prev_tex= temp.getTextureDId();
	for(uint32 i=0; i<reduced.size(); i++){
		reduced[i].bind();

		real32 factor=1.0/((real32)(reduced[i].getWidth()*2.0));
		shaders[shaderIndex]->setUniform(reductionLoc[shaderIndex], factor);
		shaders[shaderIndex]->setTexture(texLoc[shaderIndex], prev_tex, 0);
		
		mapQuad.draw();

		prev_tex= reduced[i].getTextureDId();

	}

	result.bind();
	result.setViewport();

	shaderIndex= Phase_DrawShadows;
	use();

	shaders[shaderIndex]->setTexture(texLoc[shaderIndex], reduced.back().getTextureDId(), 0);

	mapQuad.draw();

	// Blur
	for (int32 i=0; i<1; i++) {
		temp.bind();

		shaderIndex= Phase_HorBlur;
		use();
		real32 val= 1.0f/(real32)temp.getWidth();
		
		shaders[shaderIndex]->setUniform(reductionLoc[shaderIndex], val);
		shaders[shaderIndex]->setTexture(texLoc[shaderIndex], result.getTextureDId(), 0);

		mapQuad.draw();
		result.bind();

		shaderIndex= Phase_VerBlur;
		use();

		shaders[shaderIndex]->setUniform(reductionLoc[shaderIndex], val);
		shaders[shaderIndex]->setTexture(texLoc[shaderIndex], temp.getTextureDId(), 0);

		mapQuad.draw();
	}
}

} // visual
} // clover