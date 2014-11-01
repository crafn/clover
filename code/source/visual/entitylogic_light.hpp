#ifndef CLOVER_VISUAL_ENTITYLOGIC_LIGHT_HPP
#define CLOVER_VISUAL_ENTITYLOGIC_LIGHT_HPP

#include "build.hpp"
#include "entitylogic.hpp"
#include "global/eventreceiver.hpp"
#include "visual/framebuffer.hpp"

namespace clover {
namespace visual {

class LightEntityDef;

class LightEntityLogic : public EntityLogic, public global::EventReceiver {
public:
	using EntityLogic::Transform;
	
	LightEntityLogic(const LightEntityDef& def);
	virtual ~LightEntityLogic();
	
	const LightEntityDef& getDef() const;
	
	Transform getLightTransform() const;
	
	Framebuffer& getShadowMap(){ return shadowMap; }
	const Framebuffer& getShadowMap() const { return shadowMap; }
	
	Framebuffer& getShadowCasterMap(){ return shadowCasterMap; }
	const Framebuffer& getShadowCasterMap() const { return shadowCasterMap; }
	
	virtual void onEvent(global::Event& e);
	
private:
	void onShadowsStateChange(bool active);
	util::Vec2i getShadowMapSize() const;
	
	/// @todo Remove and use casted base class def ptr
	const LightEntityDef* definition;
	
	Framebuffer shadowMap;
	Framebuffer shadowCasterMap;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITYLOGIC_LIGHT_HPP