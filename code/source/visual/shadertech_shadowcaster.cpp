#include "shadertech_shadowcaster.hpp"
#include "entitylogic_light.hpp"
#include "entity_def_light.hpp"
#include "resources/cache.hpp"
#include "entity_def_model.hpp"
#include "shader.hpp"

namespace clover {
namespace visual {

ShadowCasterST::ShadowCasterST(){
	shaders.pushBack(&resources::gCache->getShaders(resources::Shader_ShadowCaster).back());
	locateUniforms(0);
}

ShadowCasterST::~ShadowCasterST(){
}

void ShadowCasterST::setLight(const LightEntityLogic& l){
	setCameraPosition(l.getLightTransform().translation.xy());
	/// @todo Calculate from intensity
	camScale= 0.15/l.getDef().getHalfValueRadius();
}

void ShadowCasterST::setEntity(const visual::ModelEntityDef& re){
	const Model* m= re.getModel();
	if (m && m->getMaterial() && m->getMaterial()->getTexture(Material::TexType_Color))
		texture= m->getMaterial()->getTexture(Material::TexType_Color)->getDId();
	else
		texture= 0;

	if (!texture) texture= resources::gCache->getResource<visual::Texture>("Texture_Black").getDId();
}

void ShadowCasterST::locateUniforms(uint32 shader_i){
	WorldShaderTech::locateUniforms(shader_i);
	textureLoc= shaders[shader_i]->getUniformLocation("uTexture");
}

void ShadowCasterST::use(){
	WorldShaderTech::use();
	shaders[0]->setTexture(textureLoc, texture, Material::TexType_Color);
}

} // visual
} // clover