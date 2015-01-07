#include "camera.hpp"
#include "entity_def_model.hpp"
#include "hardware/device.hpp"
#include "hardware/glstate.hpp"
#include "shader.hpp"
#include "shadertech.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace visual {

void WorldShaderTech::use(Shader& shd)
{
	shd.use();
	util::Vec2d aspect= global::g_env.device->getAspectVector();

	shd.setUniform("u_aspectRatio", aspect);
	shd.setUniform("u_envIntensity", envLight);
	shd.setUniform("u_envDir", envLightDir);
	shd.setUniform("u_camPos", camPos);
	shd.setUniform("u_camScale", camScale);
	shd.setUniform("u_perspectiveMul", perspectiveMul);

	if (textures[0])
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_colorMap", textures[0], Material::TexType_Color);
	if (textures[1])
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_normalMap", textures[1], Material::TexType_Normal);
	if (textures[2])
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_envShadowMap", textures[2], Material::TexType_EnvShadow);

	shd.setUniform("u_transform", transform);
}

void WorldShaderTech::setEnvLight(util::Color col, util::Vec2f dir)
{
	envLight= {col.r, col.g, col.b};
	envLightDir= dir;
}

void WorldShaderTech::setCamera(Camera& c)
{
	setCameraPosition(c.getPosition());
	camScale= c.getScale();
	perspectiveMul= c.getPerspectiveMul();
}

void WorldShaderTech::setCameraPosition(util::Vec2d pos)
{
	camPos= pos.casted<util::Vec2f>();
}

void WorldShaderTech::setCameraScale(real32 scale)
{
	camScale= scale;
}

void WorldShaderTech::setEntity(const visual::ModelEntityDef& re)
{
	PROFILE();
	const Model* model= re.getModel();
	const Texture* texs[Material::TexType_Last]= {
		nullptr, nullptr, nullptr
	};

	if (model && model->getMaterial()){
		texs[0]= model->getMaterial()->getTexture(Material::TexType_Color);
		texs[1]= model->getMaterial()->getTexture(Material::TexType_Normal);
		texs[2]= model->getMaterial()->getTexture(Material::TexType_EnvShadow);
	}

	// util::Color 0, Normal 1
	textures[0]= texs[0] ? texs[0]->getDId() : 0;
	textures[1]= texs[1] ? texs[1]->getDId() : 0;
	textures[2]= texs[2] ? texs[2]->getDId() : 0;

	if (re.isFilled())
		hardware::gGlState->setPolygonMode(GL_FILL);
	else
		hardware::gGlState->setPolygonMode(GL_LINE);
	
	hardware::gGlState->setBlendFunc(re.getBlendFunc());
}

void WorldShaderTech::setTransform(const util::SrtTransform3d& t)
{
	ensure(	std::isfinite(t.translation.x) &&
			std::isfinite(t.translation.y));

	auto&& rot= t.rotation.casted<util::Quatf>().asMatrix();
	transform=
		util::Mat44f::byScale(t.scale.casted<util::Vec3f>())*
		util::Mat44f::identity().applied(rot)*
		util::Mat44f::byTranslation(t.translation.casted<util::Vec3f>());
}

void WorldShaderTech::setColorMap(uint32 tex)
{
	textures[0]= tex;
}

void WorldShaderTech::setNormalMap(uint32 tex)
{
	textures[1]= tex;
}

} // visual
} // clover
