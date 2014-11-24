#include "shadertech.hpp"
#include "camera.hpp"
#include "hardware/device.hpp"
#include "shader.hpp"
#include "entity_def_model.hpp"
#include "hardware/glstate.hpp"

namespace clover {
namespace visual {

void WorldShaderTech::use(Shader& shd)
{
	shd.use();
	util::Vec2d aspect= hardware::gDevice->getAspectVector();

	shd.setUniform("uAspectRatio", aspect);
	shd.setUniform("uEnvIntensity", envLight);
	shd.setUniform("uEnvLightDir", envLightDir);
	shd.setUniform("uGlobalTransformation", camPos);
	shd.setUniform("uGlobalScale", camScale);

	if (textures[0])
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"uColorMap", textures[0], Material::TexType_Color);
	if (textures[1])
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"uNormalMap", textures[1], Material::TexType_Normal);
	if (textures[2])
		shd.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"uEnvShadowMap", textures[2], Material::TexType_EnvShadow);

	shd.setUniform("uTransformation", translation);
	shd.setUniform("uRotation", rotation);
	shd.setUniform("uScale", scale);
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
}

void WorldShaderTech::setCameraPosition(util::Vec2d pos)
{
	camPos= util::Vec3f{(real32)pos.x, (real32)pos.y, 0.0};
}

void WorldShaderTech::setCameraScale(real32 scale)
{
	camScale= scale;
}

void WorldShaderTech::setEntity(const visual::ModelEntityDef& re)
{
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

void WorldShaderTech::setTransformation(util::Vec2d t, util::Quatd rot)
{
	ensure(std::isfinite(translation.x) && std::isfinite(translation.y));

	translation= util::Vec3f{(real32)t.x, (real32)t.y, 0.0};
	rotation= rot.casted<util::Quatf>().asMatrix();
}

void WorldShaderTech::setScale(util::Vec2d s)
{
	scale= util::Vec3f{(real32)s.x, (real32)s.y, 0.0};
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
