#include "framebuffer.hpp"
#include "hardware/device.hpp"
#include "hardware/glstate.hpp"

namespace clover {
namespace visual {

Framebuffer::Framebuffer(Framebuffer&& other)
		: fboId(other.fboId)
		, texId(other.texId)
		, cfg(other.cfg){
	other.fboId= 0;
	other.texId= 0;
	other.depthId= 0;
}

Framebuffer::~Framebuffer(){
	destroy();
}

void Framebuffer::create(const Cfg& config){
	ensure(texId == 0);

	cfg= config;

	fboId= hardware::gGlState->genFbo();
	hardware::gGlState->bindFbo(fboId);

	// Color map

	texId= hardware::gGlState->genTex();
	hardware::GlState::TexFormat format= hardware::GlState::TexFormat::Rgb8;
	if (cfg.alpha)
		format= hardware::GlState::TexFormat::Rgba8;

	if (cfg.layers == 1){
		texTarget= hardware::GlState::TexTarget::Tex2d;
	}
	else {
		// Tex2dArray would be more suitable, but
		// OpenCL 1.1 doesn't support it
		texTarget= hardware::GlState::TexTarget::Tex3d;
	}

	hardware::gGlState->bindTex(texTarget, texId);

	util::Vec3i size{cfg.resolution.x, cfg.resolution.y, cfg.layers};
	hardware::gGlState->submitTexData(format,
							hardware::GlState::Type::Real32,
							size,
							nullptr);

	hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if (cfg.linearInterpolation){
		hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
	else {
		hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	hardware::gGlState->attachTexToFbo(GL_COLOR_ATTACHMENT0, texId);
	
	if (cfg.depth){
		// Depth map
		
		depthId= hardware::gGlState->genTex();
		
		hardware::GlState::TexTarget depth_target= texTarget;
		if (depth_target == hardware::GlState::TexTarget::Tex3d){
			// Depth tex cannot be 3d, says OpenGL
			depth_target= hardware::GlState::TexTarget::Tex2dArray;
		}

		hardware::gGlState->bindTex(depth_target, depthId);

		hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		hardware::gGlState->setTexParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		hardware::gGlState->setTexParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		hardware::gGlState->setTexParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
		hardware::gGlState->submitTexData(hardware::GlState::TexFormat::Depth,
								hardware::GlState::Type::Real32,
								size,
								nullptr);

		hardware::gGlState->attachTexToFbo(GL_DEPTH_ATTACHMENT, depthId);
	}

	GLenum status= hardware::gGlState->getFboStatus();

	if (status != GL_FRAMEBUFFER_COMPLETE)
		throw global::Exception("Framebuffer::create(..) failed: %i", status);

	hardware::gGlState->setClearColor(util::Color{0, 0, 0, 1});
	hardware::gGlState->clear(GL_COLOR_BUFFER_BIT);

	hardware::gGlState->bindFbo(0);
}

void Framebuffer::bind() const {
	if (texId == 0)
		throw(global::Exception("Framebuffer::bind(): Not initialized\n"));

	setViewport();

	hardware::gGlState->bindTex(texTarget, texId);
	hardware::gGlState->bindFbo(fboId);
}

void Framebuffer::setViewport() const {
	hardware::gGlState->setViewport(
			util::Vec2i{0, 0},
			cfg.resolution);
}

int32 Framebuffer::getWidth() const {
	return cfg.resolution.x;
}

void Framebuffer::destroy(){
	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, 0);
	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2dArray, 0);
	hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex3d, 0);

	hardware::gGlState->bindFbo(0);

	if (texId)
		hardware::gGlState->deleteTex(texId);
	if (depthId)
		hardware::gGlState->deleteTex(depthId);
	if (fboId)
		hardware::gGlState->deleteFbo(fboId);

	fboId= 0;
	texId= 0;
	depthId= 0;
}

} // visual
} // clover