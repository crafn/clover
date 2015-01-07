#include "camera.hpp"
#include "util/time.hpp"
#include "global/env.hpp"
#include "global/event.hpp"
#include "global/cfg_mgr.hpp"
#include "resources/cache.hpp"
// Debugging
#include "hardware/keyboard.hpp"

namespace clover {
namespace visual {

Camera::Camera():
			maxAcceleration(100),
			scale(0.15),
			targetScale(scale),
			perspectiveMul(1.0),
			active(true){

	global::Event e(global::Event::OnCameraCreate);
	e(global::Event::Active)= true;
	e(global::Event::Object)= this;
	e.send();
	
	Framebuffer::Cfg fbo_cfg;
	fbo_cfg.resolution= util::Vec2i(global::gCfgMgr->get<int32>("visual::defaultShadowMapSize", 512));
	fbo_cfg.linearInterpolation= true;
	shadowCasterBuf.create(fbo_cfg);

	// Entity is only to trigger fluid rendering at correct time
	fluidModel.setMesh(global::g_env->resCache->getResource<TriMesh>("unitRect"));
	fluidEntityDef.setModel(fluidModel);
	fluidEntityDef.setShadingType(ModelEntityDef::Shading_Fluid);
	fluidEntity.setDef(fluidEntityDef);
}

Camera::~Camera(){
	global::Event e(global::Event::OnCameraDestroy);
	e(global::Event::Object)= this;
	e.send();
	
	shadowCasterBuf.destroy();
}

void Camera::setPosition(util::Vec2d pos, bool override_target){
	position= pos;
	if (override_target)
		targetPosition= pos;
}

void Camera::setScale(real32 f, bool override_target){
	scale= f;
	if (override_target)
		targetScale= f;
}

void Camera::setVelocity(util::Vec2d vel){
	velocity= vel;
}

util::Vec2d Camera::getPosition() const {
	return position;
}

real32 Camera::getScale() const {
	return scale;
}

util::Vec2d Camera::getVelocity() const{
	return velocity;
}

bool Camera::isActive() const {
	return active;
}

void Camera::select(){
	global::Event e(global::Event::OnCameraSelect);
	e(global::Event::Object)= this;
	e.send();
}

bool Camera::isWorldPointInViewport(util::Vec2d p){
	util::Vec2d rad= util::Vec2d(1.0)/scale;

	if (p.componentsGreaterOrEqualThan(position-rad) && p.componentsLessThan(position+rad) )
		return true;

	return false;
}

bool Camera::isWorldBBInViewport(util::Vec2d p, util::Vec2d bb){

	util::Vec2d rad= util::Vec2d(1.0)/scale;

	if (p.componentsGreaterOrEqualThan( position-rad-bb ) && p.componentsLessThan( position+rad+bb ))
		return true;

	return false;

}


void Camera::update(){

	real32 dt= global::g_env->realClock->getDeltaTime();
	
	
	targetPosition += util::Vec2d{panningVelocity.x, panningVelocity.y}*dt;
	targetScale += panningVelocity.z*dt;

	velocity= (targetPosition - position)*10.0*dt;
	
	util::Vec2d prev_pos= position;
	
	position += velocity;
	
	if ((prev_pos.x < targetPosition.x && position.x > targetPosition.x) ||
		(prev_pos.x > targetPosition.x && position.x < targetPosition.x)){
		velocity.x= 0.0;
		position.x= targetPosition.x;
	}
	
	if ((prev_pos.y < targetPosition.y && position.y > targetPosition.y) ||
		(prev_pos.y > targetPosition.y && position.y < targetPosition.y)){
		velocity.y= 0.0;
		position.y= targetPosition.y;
	}
	
	float prev_scale= scale;
	scale = util::limited(scale + (targetScale - scale)*10.0*dt, 0.005, 1.0);
	
	if ((scale < targetScale && prev_scale > targetScale) ||
		(scale > targetScale && prev_scale < targetScale))
		scale= targetScale; // Prevent spring-like behaviour on low fps

	// Update fluid fbo if needed
	if (!fluidFbo.isCreated() || fluidFbo.getResolution() != resolution/4){
		Framebuffer::Cfg cfg;
		cfg.resolution= resolution/4;
		cfg.linearInterpolation= true;

		fluidFbo.destroy();
		fluidFbo.create(cfg);
	}

	// Fluid entity should fill the whole screen
	fluidEntity.setPosition(position.casted<util::Vec3d>());
	fluidEntity.setScale(util::Vec3d(1.0/scale));
}

Framebuffer& Camera::getShadowCasterBuf(){
	return shadowCasterBuf;
}

} // visual
} // clover
