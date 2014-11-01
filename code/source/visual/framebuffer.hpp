#ifndef CLOVER_VISUAL_FRAMEBUFFER_HPP
#define CLOVER_VISUAL_FRAMEBUFFER_HPP

#include "build.hpp"
#include "hardware/glstate.hpp"
#include "util/color.hpp"
#include "global/exception.hpp"
#include "util/misc.hpp"

#include <iostream>

namespace clover {
namespace visual {

class Framebuffer {
public:
	struct Cfg {
		util::Vec2i resolution;
		int32 layers= 1;
		bool linearInterpolation= false;
		bool alpha= false;
		bool depth= false;
	};

	Framebuffer()= default;
	Framebuffer(Framebuffer&&);
	Framebuffer(const Framebuffer&)= delete;
	~Framebuffer();

	void create(const Cfg& cfg);
	bool isCreated() const { return fboId != 0; }
	void bind() const;
	void setViewport() const;
	
	hardware::GlState::TexDId getTextureDId() const { return texId; }
	hardware::GlState::TexTarget getTexTarget() const { return texTarget; }

	util::Vec2i getResolution() const { return cfg.resolution; }
	int32 getWidth() const;
	void destroy();

private:
	hardware::GlState::FboDId fboId= 0;
	hardware::GlState::TexDId texId= 0;
	hardware::GlState::TexTarget texTarget= hardware::GlState::TexTarget::Tex2d;
	hardware::GlState::TexDId depthId= 0;
	Cfg cfg;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_FRAMEBUFFER_HPP