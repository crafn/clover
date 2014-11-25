#ifndef CLOVER_VISUAL_CAMERA_HPP
#define CLOVER_VISUAL_CAMERA_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "visual/framebuffer.hpp"
#include "visual/texture.hpp"
#include "visual/material.hpp"
#include "visual/model.hpp"
#include "visual/mesh.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/entity.hpp"

namespace clover {
namespace visual {

class Camera {
public:
	Camera();
	~Camera();

	void setTargetScale(real32 f){ targetScale= f; }
		
	void setScale(real32 f, bool override_target= true);
	real32 getScale() const;
	
	void setPosition(util::Vec2d pos, bool override_target= true);
	util::Vec2d getPosition() const;
	
	void setTargetPosition(const util::Vec2d& pos){ targetPosition= pos; }
	const util::Vec2d& getTargetPosition() const { return targetPosition; }
	
	void setVelocity(util::Vec2d vel);
	util::Vec2d getVelocity() const;

	void setTargetVelocity(util::Vec2d vel){ targetVelocity= vel; }
	void setMaxAcceleration(real32 a){ maxAcceleration= a; }

	void setPerspectiveMul(real32 mul) { perspectiveMul= mul; }
	real32 getPerspectiveMul() const { return perspectiveMul; }

	bool isActive() const;
	
	/// Sets active
	void select();

	bool isWorldPointInViewport(util::Vec2d p);
	bool isWorldBBInViewport(util::Vec2d p, util::Vec2d bb);

	void update();

	void setResolution(util::Vec2i reso){ resolution= reso; }

	Framebuffer& getShadowCasterBuf();
	Framebuffer& getFluidFbo(){ return fluidFbo; }

private:
	util::Vec2d position;
	util::Vec2d targetPosition;

	util::Vec2d velocity;
	util::Vec2d targetVelocity;
	
	real32 maxAcceleration;
	real32 scale;
	real32 targetScale;
	real32 perspectiveMul;

	util::Vec2i resolution;

	bool active;

	Framebuffer shadowCasterBuf;

	Framebuffer fluidFbo;
	Model fluidModel;
	ModelEntityDef fluidEntityDef;
	Entity fluidEntity;

	util::Vec3d panningVelocity;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_CAMERA_HPP
