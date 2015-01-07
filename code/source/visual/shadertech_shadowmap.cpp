#include "framebuffer.hpp"
#include "global/cfg_mgr.hpp"
#include "hardware/glstate.hpp"
#include "mesh.hpp"
#include "resources/cache.hpp"
#include "shader.hpp"
#include "shader_mgr.hpp"
#include "shadertech_shadowmap.hpp"
#include "shadertemplate.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace visual {

ShadowMapST::ShadowMapST(){
	int32 size= global::g_env.cfg->get<int32>("visual::defaultShadowMapSize", 512);

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
	m.addRectByCenter(
			util::Vec2f(0.0),
			util::Vec2f(1.0),
			util::Vec2f(0.5),
			util::Vec2f(0.5));
	mapQuad.add(m);
	mapQuad.flush();
}

void ShadowMapST::generate(hardware::GlState::TexDId castermap, Framebuffer& result){
	PROFILE();
	ShaderOptions opt;
	const char* state= "STATE";
	
	{ // Distort
		temp.bind();

		opt.values[state]= static_cast<int32>(Phase::distort);
		Shader& shd= getShaderMgr().getShader("visual_shadowMap", opt);
		shd.use();
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_tex", castermap, 0);
		mapQuad.draw();
	}

	{ // Reduce
		opt.values[state]= static_cast<int32>(Phase::reduction2);
		Shader& shd= getShaderMgr().getShader("visual_shadowMap", opt);
		shd.use();

		uint32 prev_tex= temp.getTextureDId();
		for(uint32 i= 0; i < reduced.size(); ++i){
			reduced[i].bind();

			real32 factor= 1.0/((real32)(reduced[i].getWidth()*2.0));
			shd.setUniform("u_reductionSize", factor);
			shd.setTexture(
					hardware::GlState::TexTarget::Tex2d,
					"u_tex", prev_tex, 0);
			mapQuad.draw();

			prev_tex= reduced[i].getTextureDId();
		}
	}

	{ // Draw shadows
		result.bind();
		result.setViewport();

		opt.values[state]= static_cast<int32>(Phase::drawShadows);
		Shader& shd= getShaderMgr().getShader("visual_shadowMap", opt);
		shd.use();
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_tex", reduced.back().getTextureDId(), 0);
		mapQuad.draw();
	}

	// Blur
	for (int32 i= 0; i < 1; i++) {
		real32 val= 1.0f/(real32)temp.getWidth();
		{
			temp.bind();

			opt.values[state]= static_cast<int32>(Phase::horBlur);
			Shader& shd= getShaderMgr().getShader("visual_shadowMap", opt);
			shd.use();
			shd.setUniform("u_reductionSize", val);
			shd.setTexture(
					hardware::GlState::TexTarget::Tex2d,
					"u_tex", result.getTextureDId(), 0);
			mapQuad.draw();
		}

		{
			result.bind();

			opt.values[state]= static_cast<int32>(Phase::verBlur);
			Shader& shd= getShaderMgr().getShader("visual_shadowMap", opt);
			shd.use();
			shd.setUniform("u_reductionSize", val);
			shd.setTexture(
					hardware::GlState::TexTarget::Tex2d,
					"u_tex", temp.getTextureDId(), 0);
			mapQuad.draw();
		}
	}
}

} // visual
} // clover
