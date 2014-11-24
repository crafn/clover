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
	void generate(hardware::GlState::TexDId castermap, Framebuffer& result);

protected:
	enum class Phase {
		distort= 0,
		reduction8,
		reduction2,
		drawShadows,
		horBlur,
		verBlur
	};

	Framebuffer temp;
	util::DynArray<Framebuffer> reduced;

	TriMesh mapQuad;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_SHADOWMAP_HPP
