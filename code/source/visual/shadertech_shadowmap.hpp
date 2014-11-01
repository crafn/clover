#ifndef CLOVER_VISUAL_SHADERTECH_SHADOWMAP_HPP
#define CLOVER_VISUAL_SHADERTECH_SHADOWMAP_HPP

#include "shadertech.hpp"
#include "framebuffer.hpp"
#include "mesh.hpp"
#include "hardware/glstate.hpp"

namespace clover {
namespace visual {

class ShadowMapST : public ShaderTech {
public:
	ShadowMapST();
	virtual ~ShadowMapST();

	void generate(hardware::GlState::TexDId castermap, Framebuffer& result);

protected:

	enum Phase {
		Phase_Distort=0,
		Phase_Reduction8,
		Phase_Reduction2,
		Phase_DrawShadows,
		Phase_HorBlur,
		Phase_VerBlur
	};
	void locateUniforms(uint32 shader_i);

	util::DynArray<uint32> texLoc;
	util::DynArray<uint32> reductionLoc;

	Framebuffer temp;
	util::DynArray<Framebuffer> reduced;

	TriMesh mapQuad;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_SHADOWMAP_HPP