#include "shadertech_generic.hpp"
#include "hardware/glstate.hpp"
#include "entitylogic_light.hpp"
#include "entity_def_light.hpp"
#include "entity_def_model.hpp"
#include "entitylogic_model.hpp"
#include "shader.hpp"
#include "shader_mgr.hpp"
#include "shadertemplate.hpp"
#include "util/time.hpp"

namespace clover {
namespace visual {

void GenericST::setLightCount(int32 lightsc)
{ lightCount= lightsc; }

void GenericST::setLights(const util::DynArray<LightEntityLogic*>& l)
{ lights= &l; }

void GenericST::setEntity(const visual::ModelEntityDef& re, const visual::ModelEntityLogic& logic)
{
	PROFILE();
	WorldShaderTech::setEntity(re);

	colorMap= textures[0];
	normalMap= textures[1];
	envShadowMap= textures[2];
	sway= re.isSwaying();
	swayPhaseMul= re.getSwayPhaseMul();
	swayScale= re.getSwayScale();
	lightAlphaAdd= re.getLightAlphaAdd();
	colorMul= re.getColorMul()*logic.getColorMul();
}

void GenericST::use()
{
	PROFILE();

	visual::ShaderOptions opt;
	if (colorMap)
		opt.defines.insert("COLORMAP");
	if (envShadowMap)
		opt.defines.insert("ENVSHADOWMAP");
	if (normalMap)
		opt.defines.insert("NORMALMAP");
	if (sway)
		opt.defines.insert("SWAY");
	if (lightCount > 0){
		opt.defines.insert("DYNAMIC_LIGHTING");
		opt.values["LIGHT_COUNT"]= lightCount;
	}
	Shader& shader= getShaderMgr().getShader("visual_generic", opt);

	WorldShaderTech::use(shader);

	shader.setUniform("uColorMul", colorMul);

	if (sway){
		real32 phase= util::gGameClock->getTime()*swayPhaseMul;
		shader.setUniform("uSwayPhase", phase);
		shader.setUniform("uSwayScale", swayScale);
	}

	// Lights
	if (lightCount > 0){
		SizeType count= lightCount;

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

		shader.setUniform("uLightPosition", *pos, count, 2);
		shader.setUniform("uLightIntensity", *intensity, count);
		shader.setUniform("uLightRange", *range, count);
		shader.setUniform("uLightRotMatrix", *rotation_matrix, count, 4);
		shader.setUniform("uLightSpread", *spread, count);
		shader.setUniform("uShadowsEnabled", *shadows_enabled, count);
		shader.setUniform("uLightAlphaAdd", lightAlphaAdd);

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

		shader.setUniform("uShadowMap", *shadow_maps, maxShadowLights);
	}
}

} // visual
} // clover
