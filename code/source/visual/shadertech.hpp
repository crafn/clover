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
class ShaderMgr;

/// Contains information and resources for running a Shader with
/// proper inputs and storing results
/// @todo Refactor whole ShaderTech hierarchy to be component based
class ShaderTech {
public:
	virtual ~ShaderTech() {};

	void setShaderMgr(ShaderMgr& mgr) { shaderMgr= &mgr; }
	ShaderMgr& getShaderMgr() const { return *NONULL(shaderMgr); }

private:
	ShaderMgr* shaderMgr= nullptr;
};

class Camera;
class ModelEntityDef;

class WorldShaderTech : public ShaderTech {
public:
	void setEnvLight(util::Color col, util::Vec2f dir);

	void setCamera(Camera& c);
	void setCameraPosition(util::Vec2d pos);
	void setCameraScale(real32 scale);

	void setEntity(const visual::ModelEntityDef& re);
	void setTransformation(util::Vec2d, util::Quatd rot);
	void setScale(util::Vec2d scale);
	void setColorMap(uint32 tex);
	void setNormalMap(uint32 tex);

protected:
	void use(Shader& shd);

	util::Vec3f envLight;
	util::Vec2f envLightDir;
	util::Vec3f camPos;
	real32 camScale;
	util::Vec3f translation;
	util::Mat33f rotation;
	util::Vec3f scale;
	util::Vec2d aspect;

	uint32 textures[Material::TexType_Last]= { 0 };
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_HPP
