#include "entitylogic_light.hpp"
#include "entity_def_light.hpp"
#include "global/event.hpp"
#include "global/cfg_mgr.hpp"

namespace clover {
namespace visual {

LightEntityLogic::LightEntityLogic(const LightEntityDef& def):
	EntityLogic(def),
	definition(&def){
	
	if (definition->hasShadows()){
		onShadowsStateChange(true);
	}

	listenForEvent(global::Event::OnLightShadowsActivation);
}

LightEntityLogic::~LightEntityLogic(){
}

const LightEntityDef& LightEntityLogic::getDef() const {
	ensure(definition);
	return *definition;
}

LightEntityLogic::Transform LightEntityLogic::getLightTransform() const {
	return getDef().getOffset()*getTransform();
}

void LightEntityLogic::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnLightShadowsActivation:
			if (e(global::Event::Object).getPtr<LightEntityDef>() == definition){
				onShadowsStateChange(e(global::Event::Active).getI());
			}
		break;
		default:;
	}
}

void LightEntityLogic::onShadowsStateChange(bool active){
	if (active){
		Framebuffer::Cfg fbo_cfg;
		fbo_cfg.resolution= getShadowMapSize();
		fbo_cfg.linearInterpolation= true;
		shadowMap.create(fbo_cfg);
		shadowCasterMap.create(fbo_cfg);

		shadowCasterMap.bind();
		hardware::gGlState->setClearColor(util::Color{1.0, 0.0, 0.0, 1.0});
		hardware::gGlState->clear(GL_COLOR_BUFFER_BIT);

		shadowMap.bind();
		hardware::gGlState->setClearColor(util::Color{1.0, 1.0, 1.0, 1.0});
		hardware::gGlState->clear(GL_COLOR_BUFFER_BIT);

		//print(debug::Ch::General, debug::Vb::Moderate, "Generate");
	}
	else {
		//shadowMap.destroy();
		//shadowCasterMap.destroy();

		//print(debug::Ch::General, debug::Vb::Moderate, "Destroy");
	}
}

util::Vec2i LightEntityLogic::getShadowMapSize() const {
	return util::Vec2i(global::gCfgMgr->get<int32>("visual::defaultShadowMapSize", 512));
}

} // visual
} // clover