#ifndef CLOVER_VISUAL_SHADERTECH_GENERIC_HPP
#define CLOVER_VISUAL_SHADERTECH_GENERIC_HPP

#include "build.hpp"
#include "shadertech.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace visual {

class LightEntityLogic;
class ModelEntityLogic;

class GenericST : public WorldShaderTech {
public:
	void setLightCount(int32 lights);
	void setEntity(const visual::ModelEntityDef& re, const visual::ModelEntityLogic& logic_cfg);
	void setLights(const util::DynArray<LightEntityLogic*>& lights);
	
	virtual void use();

private:
	uint32 colorMap= 0;
	uint32 normalMap= 0;
	uint32 envShadowMap= 0;
	bool sway= false;
	bool disableEnv= false;

	real32 lightAlphaAdd= 0.0;
	util::Color colorMul;
	real32 swayPhaseMul= 0.0;
	real32 swayScale= 0.0;

	const util::DynArray<LightEntityLogic*>* lights= nullptr;
	int32 lightCount= 0;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_GENERIC_HPP
