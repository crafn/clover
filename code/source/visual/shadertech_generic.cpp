#include "shadertech_generic.hpp"
#include "hardware/glstate.hpp"
#include "entitylogic_light.hpp"
#include "entity_def_light.hpp"
#include "entity_def_model.hpp"
#include "entitylogic_model.hpp"
#include "resources/cache.hpp"
#include "shader.hpp"
#include "util/time.hpp"

namespace clover {
namespace visual {

void GenericST::locateUniforms(uint32 shader_i){
	if (shader_i >= lightPosLoc.size()){
		lightPosLoc.resize(shader_i+1);
		lightIntensityLoc.resize(shader_i+1);
		lightRangeLoc.resize(shader_i+1);
		lightRotMatrixLoc.resize(shader_i+1);
		lightSpreadLoc.resize(shader_i+1);
		shadowsEnabledLoc.resize(shader_i+1);
		shadowMapLoc.resize(shader_i+1);
		lightAlphaAddLoc.resize(shader_i+1);
		colorMulLoc.resize(shader_i+1);
		swayPhaseLoc.resize(shader_i+1);
		swayScaleLoc.resize(shader_i+1);
	}
	
	lightPosLoc[shader_i]= shaders[shader_i]->getUniformLocation("uLightPosition");
	lightIntensityLoc[shader_i]= shaders[shader_i]->getUniformLocation("uLightIntensity");
	lightRangeLoc[shader_i]= shaders[shader_i]->getUniformLocation("uLightRange");
	lightRotMatrixLoc[shader_i]= shaders[shader_i]->getUniformLocation("uLightRotMatrix");
	lightSpreadLoc[shader_i]= shaders[shader_i]->getUniformLocation("uLightSpread");
	shadowsEnabledLoc[shader_i]= shaders[shader_i]->getUniformLocation("uShadowsEnabled");
	shadowMapLoc[shader_i]= shaders[shader_i]->getUniformLocation("uShadowMap");
	lightAlphaAddLoc[shader_i]= shaders[shader_i]->getUniformLocation("uLightAlphaAdd");
	colorMulLoc[shader_i]= shaders[shader_i]->getUniformLocation("uColorMul");
	swayPhaseLoc[shader_i]= shaders[shader_i]->getUniformLocation("uSwayPhase");
	swayScaleLoc[shader_i]= shaders[shader_i]->getUniformLocation("uSwayScale");

	WorldShaderTech::locateUniforms(shader_i);
}

GenericST::GenericST()
		: lightAlphaAdd(0.0)
		, lights(0){
	shaderIndex= 0;
}

GenericST::~GenericST(){
}

void GenericST::setLightCount(int32 lightsc){		
	curType.lightCount= lightsc;
}

void GenericST::setLights(const util::DynArray<LightEntityLogic*>& l){
	lights= &l;
}

void GenericST::setEntity(const visual::ModelEntityDef& re, const visual::ModelEntityLogic& logic){
	PROFILE();
	WorldShaderTech::setEntity(re);

	curType.colorMap= textures[0];
	curType.normalMap= textures[1];
	curType.envShadowMap= textures[2];

	curType.curve= re.getSmoothType() == visual::ModelEntityDef::Smooth_TriCurve?true:false;
	curType.sway= re.isSwaying();

	swayPhaseMul= re.getSwayPhaseMul();
	swayScale= re.getSwayScale();

	lightAlphaAdd= re.getLightAlphaAdd();
	
	colorMul= re.getColorMul()*logic.getColorMul();
}

void GenericST::use(){
	PROFILE();
	if (!(curType == lastType) || shaders.empty()){
		uint32 id= curType.getId();

		if (shaders.size() <= id)
			shaders.resize(id+1, 0);
			
		shaderIndex= id;
		if (!shaders[id]){
			shaders[id]= &resources::gCache->getGenericShader(curType);
			locateUniforms(curType.getId());
		}
	}
	Shader& shader= *shaders[shaderIndex];
	
	WorldShaderTech::use();

	shaders[shaderIndex]->setUniform(colorMulLoc[shaderIndex], colorMul);

	if (curType.sway){
		real32 phase= util::gGameClock->getTime()*swayPhaseMul;
		shader.setUniform(swayPhaseLoc[shaderIndex], phase);
		shader.setUniform(swayScaleLoc[shaderIndex], swayScale);
	}

	// Lights
	if (curType.lightCount > 0){
		SizeType count= curType.lightCount;

		if (count != 0 && !lights)
			throw global::Exception("GenericST::use(): lights not set (count: %i)", count);

		if (count != lights->size())
			throw global::Exception("GenericST::use(): wrong light count (count: %i, promised: %i)", count, lights->size());

		real32 pos[2*count];
		real32 intensity[count];
		real32 range[count];

		real32 rotation_matrix[4*count]; // 2x2 matriisi
		real32 spread[count];

		int32 shadows_enabled[count];
		
		for (SizeType i= 0; i < count; i++){
			const LightEntityLogic* light= (*lights)[i];
			
			util::Vec2d position= light->getLightTransform().translation.xy();
			pos[2*i] = position.x;
			pos[2*i+1] = position.y;

			intensity[i] = light->getDef().getIntensity();
			range[i] = light->getDef().getHalfValueRadius();
			spread[i] = 1.0;

			shadows_enabled[i]= light->getDef().hasShadows();
			
			rotation_matrix[4*i]= cos(light->getRotation().rotationZ());
			rotation_matrix[4*i+1]= -sin(light->getRotation().rotationZ()); 
			rotation_matrix[4*i+2]= sin(light->getRotation().rotationZ());
			rotation_matrix[4*i+3]= cos(light->getRotation().rotationZ());
		}

		shader.setUniform(lightPosLoc[shaderIndex], *pos, count, 2);
		shader.setUniform(lightIntensityLoc[shaderIndex], *intensity, count);
		shader.setUniform(lightRangeLoc[shaderIndex], *range, count);
		shader.setUniform(lightRotMatrixLoc[shaderIndex], *rotation_matrix, count, 4); // En tiiä meneekö oikein kun en oo rotaatiota testannu vielä
		shader.setUniform(lightSpreadLoc[shaderIndex], *spread, count);
		shader.setUniform(shadowsEnabledLoc[shaderIndex], *shadows_enabled, count);
		shader.setUniform(lightAlphaAddLoc[shaderIndex], lightAlphaAdd);

		//shader.uniform1iv(shadowsEnabledLoc, count, false);
		int32 maxShadowLights=count;
		if (maxShadowLights>13)maxShadowLights= 13;
		int32 shadow_maps[maxShadowLights];
		int32 cur_map=0;

		// Shadowmaps
		for (auto it= lights->begin(); it!= lights->end(); it++){
			const LightEntityLogic* light= (*it);
			if (light->getDef().hasShadows()){
				shadow_maps[cur_map]= cur_map + 3;
				hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, light->getShadowMap().getTextureDId(), cur_map + 3);
			}
			else {
				shadow_maps[cur_map]= 0;
			}
			cur_map ++;
		}
		
		for (int32 i=cur_map; i<maxShadowLights; i++){
			hardware::gGlState->bindTex(hardware::GlState::TexTarget::Tex2d, resources::gCache->getResource<visual::Texture>("Texture_Empty").getDId(), cur_map + 3);
			shadow_maps[i]=0;
		}

		shader.setUniform(shadowMapLoc[shaderIndex], *shadow_maps, maxShadowLights);
	}
	lastType= curType;
}

} // visual
} // clover