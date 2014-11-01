#include "shadertech.hpp"
#include "camera.hpp"
#include "hardware/device.hpp"
#include "shader.hpp"
#include "entity_def_model.hpp"
#include "hardware/glstate.hpp"

namespace clover {
namespace visual {

ShaderTech::ShaderTech()
		: shaderIndex(0){
}

ShaderTech::~ShaderTech(){
}

void ShaderTech::use(){
	shaders[shaderIndex]->use();
}

WorldShaderTech::WorldShaderTech(){
	for (auto &m : textures)
		m= 0;
}

WorldShaderTech::~WorldShaderTech(){
}

void WorldShaderTech::locateUniforms(uint32 shader_i){
	/// This is bad.
	
	if (shader_i >= envLightLoc.size()){
		envLightLoc.resize(shader_i+1);
		envLightDirLoc.resize(shader_i+1);
		camPosLoc.resize(shader_i+1);
		camScaleLoc.resize(shader_i+1);
		translationLoc.resize(shader_i+1);
		rotationLoc.resize(shader_i+1);
		scaleLoc.resize(shader_i+1);
		aspectLoc.resize(shader_i+1);
		colorMapLoc.resize(shader_i+1);
		normalMapLoc.resize(shader_i+1);
		envShadowMapLoc.resize(shader_i+1);
	}
	
	envLightLoc[shader_i]= shaders[shader_i]->getUniformLocation("uEnvIntensity");
	envLightDirLoc[shader_i]= shaders[shader_i]->getUniformLocation("uEnvLightDir");
	camPosLoc[shader_i]= shaders[shader_i]->getUniformLocation("uGlobalTransformation");
	camScaleLoc[shader_i]= shaders[shader_i]->getUniformLocation("uGlobalScale");
	translationLoc[shader_i]= shaders[shader_i]->getUniformLocation("uTransformation");
	rotationLoc[shader_i]= shaders[shader_i]->getUniformLocation("uRotation");
	scaleLoc[shader_i]= shaders[shader_i]->getUniformLocation("uScale");
	aspectLoc[shader_i]= shaders[shader_i]->getUniformLocation("uAspectRatio");
	colorMapLoc[shader_i]= shaders[shader_i]->getUniformLocation("uColorMap");
	normalMapLoc[shader_i]= shaders[shader_i]->getUniformLocation("uNormalMap");
	envShadowMapLoc[shader_i]= shaders[shader_i]->getUniformLocation("uEnvShadowMap");
}

void WorldShaderTech::use(){
	ShaderTech::use();

	int32 i=shaderIndex;

	util::Vec2d aspect= hardware::gDevice->getAspectVector();

	shaders[i]->setUniform(aspectLoc[i], aspect);
	shaders[shaderIndex]->setUniform(envLightLoc[shaderIndex], envLight);
	shaders[shaderIndex]->setUniform(envLightDirLoc[shaderIndex], envLightDir);
	shaders[shaderIndex]->setUniform(camScaleLoc[shaderIndex], camScale);
	shaders[shaderIndex]->setUniform(camPosLoc[shaderIndex], camPos);

	if (textures[0])
		shaders[shaderIndex]->setTexture(colorMapLoc[shaderIndex], textures[0], Material::TexType_Color);
	if (textures[1])
		shaders[shaderIndex]->setTexture(normalMapLoc[shaderIndex], textures[1], Material::TexType_Normal);
	if (textures[2])
		shaders[shaderIndex]->setTexture(envShadowMapLoc[shaderIndex], textures[2], Material::TexType_EnvShadow);

	shaders[shaderIndex]->setUniform(translationLoc[shaderIndex], translation);
	shaders[shaderIndex]->setUniform(rotationLoc[shaderIndex], rotation);
	shaders[shaderIndex]->setUniform(scaleLoc[shaderIndex], scale);
}

void WorldShaderTech::setEnvLight(util::Color col, util::Vec2f dir){
	envLight= {col.r, col.g, col.b};
	envLightDir= dir;
}

void WorldShaderTech::setCamera(Camera& c){
	setCameraPosition(c.getPosition());
	camScale= c.getScale();
}

void WorldShaderTech::setCameraPosition(util::Vec2d pos){
	camPos= util::Vec3f{(real32)pos.x, (real32)pos.y, 0.0};
}

void WorldShaderTech::setCameraScale(real32 scale){
	camScale= scale;
}

void WorldShaderTech::setEntity(const visual::ModelEntityDef& re){
	const Model* model= re.getModel();
	
	const Texture* texs[Material::TexType_Last]= {
		0,
		0,
		0
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

void WorldShaderTech::setTransformation(util::Vec2d t, util::Quatd rot){
	ensure(std::isfinite(translation.x) && std::isfinite(translation.y));

	translation= util::Vec3f{(real32)t.x, (real32)t.y, 0.0};
	rotation= rot.casted<util::Quatf>().asMatrix();
}

void WorldShaderTech::setScale(util::Vec2d s){
	scale= util::Vec3f{(real32)s.x, (real32)s.y, 0.0};
}

void WorldShaderTech::setColorMap(uint32 tex){
	textures[0]= tex;
}

void WorldShaderTech::setNormalMap(uint32 tex){
	textures[1]= tex;
}

} // visual
} // clover