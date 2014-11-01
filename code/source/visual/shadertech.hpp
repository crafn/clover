#ifndef CLOVER_VISUAL_SHADERTECH_HPP
#define CLOVER_VISUAL_SHADERTECH_HPP

#include "build.hpp"
#include "util/math.hpp"
#include "util/dyn_array.hpp"
#include "visual/material.hpp"
#include "visual/texture.hpp"

namespace clover {
namespace visual {

class Shader;

/// @todo Refactor whole ShaderTech hierarchy to be component based
/// Contains information and resources for running a Shader with
/// proper inputs and storing results
class ShaderTech {
public:
	ShaderTech();
	virtual ~ShaderTech();

	virtual void use();

protected:
	util::DynArray<Shader*> shaders;

	// use() will turn on this shader
	int32 shaderIndex;

	virtual void locateUniforms(uint32 shader_i)=0;
};

class Camera;
class ModelEntityDef;

class WorldShaderTech : public ShaderTech {
public:
	WorldShaderTech();
	virtual ~WorldShaderTech();

	void setEnvLight(util::Color col, util::Vec2f dir);

	void setCamera(Camera& c);
	void setCameraPosition(util::Vec2d pos);
	void setCameraScale(real32 scale);

	void setEntity(const visual::ModelEntityDef& re);
	void setTransformation(util::Vec2d, util::Quatd rot);
	void setScale(util::Vec2d scale);
	void setColorMap(uint32 tex);
	void setNormalMap(uint32 tex);

	virtual void use();

protected:
	virtual void locateUniforms(uint32 shader_i);

protected:
	util::Vec3f envLight;
	util::Vec2f envLightDir;
	util::Vec3f camPos;
	real32 camScale;
	util::Vec3f translation;
	util::Mat33f rotation;
	util::Vec3f scale;
	util::Vec2d aspect;

	uint32 textures[Material::TexType_Last];

	util::DynArray<int32> envLightLoc,
					envLightDirLoc,
					camPosLoc,
					camScaleLoc,
					translationLoc,
					rotationLoc,
					scaleLoc,
					aspectLoc,
					colorMapLoc,
					normalMapLoc,
					envShadowMapLoc;

};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_HPP