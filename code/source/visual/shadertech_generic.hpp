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
	GenericST();
	virtual ~GenericST();

	void setLightCount(int32 lights);
	void setEntity(const visual::ModelEntityDef& re, const visual::ModelEntityLogic& logic_cfg);
	void setLights(const util::DynArray<LightEntityLogic*>& lights);
	
	virtual void use();

private:
	util::DynArray<int32> 	lightPosLoc,
						lightIntensityLoc,
						lightRangeLoc,
						lightRotMatrixLoc,
						lightSpreadLoc,
						shadowsEnabledLoc,
						shadowMapLoc,
						lightAlphaAddLoc,
						colorMulLoc,
						swayPhaseLoc,
						swayScaleLoc;

	virtual void locateUniforms(uint32 shader_i);

	bool disableEnv;
	bool useCurve;

	real32 lightAlphaAdd;
	util::Color colorMul;
	real32 swayPhaseMul;
	real32 swayScale;

	const util::DynArray<LightEntityLogic*>* lights;

	resources::Cache::GenericShaderType curType, lastType;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_GENERIC_HPP