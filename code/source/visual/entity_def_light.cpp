#include "entity_def_light.hpp"

namespace clover {
namespace visual {

LightEntityDef::LightEntityDef(real32 dist, real32 intens, bool shadows):
		INIT_RESOURCE_ATTRIBUTE(radiusAttribute, "halfValueRadius", dist),
		INIT_RESOURCE_ATTRIBUTE(intensityAttribute, "intensity", intens),
		INIT_RESOURCE_ATTRIBUTE(shadowsAttribute, "shadows", shadows){
	type= EntityDef::Type::Light;

	shadowsAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			global::Event e(global::Event::OnLightShadowsActivation);
			e(global::Event::Active)= shadowsAttribute.get();
			e(global::Event::Object)= this;
			e.send();
		}
	});
}

LightEntityDef::~LightEntityDef(){
}

LightEntityLogic* LightEntityDef::createLogic() const {
	return new LightEntityLogic(*this);
}

void LightEntityDef::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void LightEntityDef::createErrorResource(){
	/// @todo
	setResourceState(State::Error);
}

} // visual
} // clover