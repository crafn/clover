#include "entity_def_model.hpp"
#include "entitylogic_model.hpp"
#include "global/env.hpp"
#include "resources/cache.hpp"
#include "util/profiling.hpp"
#include "visual/model.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace visual {

ModelEntityDef::ModelEntityDef()
			: shadingType(Shading_Generic)
			, filled(true)
			, snapToPixels(false)
			, colorMul({1,1,1,1})
			, lightAlphaAdd(0.0)
			, model(0)
			, drawPriority(0)
			, sway(0)
			, swayPhaseMul(1)
			, swayScale(1)
			, INIT_RESOURCE_ATTRIBUTE(modelAttribute, "model", "")
			, INIT_RESOURCE_ATTRIBUTE(envLightAttribute, "envLight", -1.0)
			, INIT_RESOURCE_ATTRIBUTE(onlyEnvAttribute, "usesOnlyEnvLight", false)
			, INIT_RESOURCE_ATTRIBUTE(castsShadowAttribute, "shadowCasting", false)
			, INIT_RESOURCE_ATTRIBUTE(billboardAttribute, "billboard", false){
	type= Type::Model;

	modelAttribute.setOnChangeCallback([&] () {
		if (getResourceState() != State::Uninit)
			model= &global::g_env->resCache->getResource<visual::Model>(modelAttribute.get());
	});
}

ModelEntityDef::~ModelEntityDef(){
}

void ModelEntityDef::setModel(const Model& m){
	model= &m;
}

void ModelEntityDef::setModel(const util::Str8& str){
	model= &global::g_env->resCache->getResource<visual::Model>(str);
}

void ModelEntityDef::setShadingType(ShadingType shdtype){
	shadingType= shdtype;
}

ModelEntityDef::ShadingType ModelEntityDef::getShadingType() const {
	return shadingType;
}

void ModelEntityDef::setEnvLight(real32 intensity){
	envLightAttribute.set(intensity);
}

bool ModelEntityDef::usesCustomEnvLight() const {
	if (envLightAttribute.get() < 0)
		return false;
	return true;
}

real32 ModelEntityDef::getCustomEnvLight() const {
	return envLightAttribute.get();
}

util::Color ModelEntityDef::getColorMul() const {
	if (model && model->getMaterial()){
		return util::Color(colorMul * model->getMaterial()->getColor());
	}
	return colorMul;
}

void ModelEntityDef::useOnlyEnvLight(bool e){
	onlyEnvAttribute.set(e);
}

bool ModelEntityDef::usesOnlyEnvLight() const {
	return onlyEnvAttribute.get();
}


void ModelEntityDef::setShadowCasting(bool b){
	castsShadowAttribute.set(b);
}

bool ModelEntityDef::isShadowCasting() const {
	return castsShadowAttribute.get();
}

void ModelEntityDef::setLightAlphaAdd(real32 f){
	lightAlphaAdd= f;
}

real32 ModelEntityDef::getLightAlphaAdd() const {
	return lightAlphaAdd;
}

void ModelEntityDef::setFilled(bool fill){
	filled= fill;
}

bool ModelEntityDef::isFilled() const {
	return filled;
}

void ModelEntityDef::draw() const {
	PROFILE();
	if (model && model->getMesh()){
		PROFILE();
		model->getMesh()->draw();
	}
}

EntityLogic* ModelEntityDef::createLogic() const {
	return new ModelEntityLogic(*this);
}

void ModelEntityDef::resourceUpdate(bool load, bool force){
	
	if (load || getResourceState() == State::Uninit){
		model= &global::g_env->resCache->getResource<Model>(modelAttribute.get());
		setResourceState(State::Loaded);
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void ModelEntityDef::createErrorResource(){
	setResourceState(State::Error);
	
	model= &global::g_env->resCache->getErrorResource<Model>();
	envLightAttribute.set(1.0);
	onlyEnvAttribute.set(true);
}

uint32 ModelEntityDef::getContentHash() const {
	if (!model)
		return 0;
	else
		return model->getContentHash() + getBatchCompatibilityHash() + util::hash32(colorMul);
}

uint32 ModelEntityDef::getBatchCompatibilityHash() const {
	if (!model)
		return 0;
	else
		return	model->getBatchCompatibilityHash()
				+ util::hash32(envLightAttribute.get())
				+ util::hash32(onlyEnvAttribute.get())*2 +
				+ util::hash32(castsShadowAttribute.get())*4
				+ util::hash32(sway)*8
				+ util::hash32(filled)*16;
}

} // visual
} // clover
