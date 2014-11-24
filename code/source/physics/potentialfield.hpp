#ifndef CLOVER_PHYSICS_POTENTIALFIELD_HPP
#define CLOVER_PHYSICS_POTENTIALFIELD_HPP

#include "build.hpp"
#include "fluidparticle.hpp"
#include "visual/framebuffer.hpp"
#include "visual/mesh.hpp"
#include "visual/shader.hpp"

namespace clover {
namespace physics {

/// Potential field for fluids
/// @note Uses OpenGL for calculations
class PotentialField {
public:
	PotentialField(uint32 chunk_reso, uint32 chunks);
	void update(uint32 chunk_size,
				const visual::PointMesh<FluidParticle>& particles,
				hardware::GlState::TexDId chunk_info_tex);

	const visual::Framebuffer& getFbo() const { return fbo; }

private:
	visual::Shader shader;
	visual::Framebuffer fbo;
	uint32 chunkCount;
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_POTENTIALFIELD_HPP
