#include "entitylogic_light.hpp"
#include "entity_def_light.hpp"
#include "entity_def_model.hpp"
#include "global/env.hpp"
#include "resources/cache.hpp"
#include "shader.hpp"
#include "shader_mgr.hpp"
#include "shadertech_shadowcaster.hpp"

namespace clover {
namespace visual {

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

	if (!texture)
		texture= global::g_env->resCache->getResource<visual::Texture>("Texture_Black").getDId();
}

void ShadowCasterST::use(){
	Shader& shd= getShaderMgr().getShader("visual_shadowCaster");
	WorldShaderTech::use(shd);
	shd.setTexture(
			hardware::GlState::TexTarget::Tex2d,
			"u_tex", texture, Material::TexType_Color);
}

} // visual
} // clover
