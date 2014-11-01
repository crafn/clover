#ifndef CLOVER_VISUAL_SHADERTECH_SHADOWCASTER_HPP
#define CLOVER_VISUAL_SHADERTECH_SHADOWCASTER_HPP

#include "build.hpp"
#include "shadertech.hpp"

namespace clover {
namespace visual {

class LightEntityLogic;
class ShadowCasterST : public WorldShaderTech {
public:
	ShadowCasterST();
	virtual ~ShadowCasterST();

	void setLight(const LightEntityLogic& l);
	void setEntity(const visual::ModelEntityDef& re);

	virtual void use();
private:
	int32 textureLoc;
	uint32 texture;
	
	void locateUniforms(uint32 shader_i);
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_SHADOWCASTER_HPP
