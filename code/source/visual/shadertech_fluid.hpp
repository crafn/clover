#ifndef CLOVER_VISUAL_SHADERTECH_FLUID_HPP
#define CLOVER_VISUAL_SHADERTECH_FLUID_HPP

#include "build.hpp"
#include "shader.hpp"
#include "shadertech.hpp"

namespace clover {
namespace visual {

class TriMesh;

class FluidST : public WorldShaderTech {
public:
	using Base= WorldShaderTech;
	FluidST();

	void render(Camera& cam);

private:
	virtual void use();

	// Draws single particles
	struct DebugDraw {
		DebugDraw();

		Shader shader;
		uint32 chunkSizeLoc;
		uint32 chunkCountLoc;
		uint32 chunkInfoSamplerLoc;
	};

	// Draws particles to camera fbo
	struct ParticleDraw {
		ParticleDraw();

		Shader shader;
		uint32 chunkSizeLoc;
		uint32 chunkCountLoc;
		uint32 chunkInfoSamplerLoc;
		uint32 timeOffsetLoc;
		uint32 hueCycleLoc;
	};

	// Draws distance field to camera fbo
	struct DistFieldDraw {
		DistFieldDraw();

		Shader shader;
		uint32 texLoc;
	};

	// Renders fluid from camera fbo
	struct PostProcess {
		PostProcess();

		Shader shader;
		uint32 texLoc;
	};

	DebugDraw debugDraw;
	ParticleDraw particleDraw;
	DistFieldDraw distFieldDraw;
	PostProcess postProcess;

	const TriMesh* quad= nullptr;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_SHADERTECH_FLUID_HPP