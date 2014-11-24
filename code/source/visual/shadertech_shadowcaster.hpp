#ifndef CLOVER_VISUAL_SHADERTECH_SHADOWCASTER_HPP
#define CLOVER_VISUAL_SHADERTECH_SHADOWCASTER_HPP

#include "build.hpp"
#include "shadertech.hpp"

namespace clover {
namespace visual {

class LightEntityLogic;
class ShadowCasterST : public WorldShaderTech {
public:
	void setLight(const LightEntityLogic& l);
	void setEntity(const visual::ModelEntityDef& re);

	void use();
private:
	uint32 texture;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_SHADOWCASTER_HPP
