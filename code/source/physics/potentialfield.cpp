#include "potentialfield.hpp"
#include "global/file.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace physics {

PotentialField::PotentialField(uint32 chunk_reso, uint32 chunks)
		: chunkCount(chunks){
	const util::Str8 shd_path= "fluids/fluidpotential";
	shader.setSources(
			global::File::readText(shd_path + ".vert"),
			global::File::readText(shd_path + ".geom"),
			global::File::readText(shd_path + ".frag"));
	shader.compile<FluidParticle>();

	visual::Framebuffer::Cfg fbo_cfg;
	fbo_cfg.resolution= util::Vec2i(chunk_reso);
	fbo_cfg.layers= chunkCount;
	fbo_cfg.linearInterpolation= true;
	fbo_cfg.alpha= true;
	fbo.create(fbo_cfg);

}

void PotentialField::update(
		uint32 chunk_size,
		const visual::PointMesh<FluidParticle>& particles,
		hardware::GlState::TexDId chunk_info_tex){
	PROFILE();

	// Render the potential field of particles
	
	// "reset"
	hardware::gGlState->setBlendFunc(hardware::GlState::BlendFunc{GL_ZERO, GL_ZERO});
	/// @todo Move to hardware::GlState
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

	fbo.bind();
	// {vel.x, vel.y, pressure, pot}
	hardware::gGlState->setClearColor({0.5, 0.5, 0, 0});
	hardware::gGlState->clear(GL_COLOR_BUFFER_BIT);
	shader.use();
	shader.setUniform("u_chunkSize", chunk_size);
	shader.setUniform("u_chunkCount", chunkCount);
	shader.setTexture(	hardware::GlState::TexTarget::Tex2d,
						"u_chunkInfo",
						chunk_info_tex,
						0);

	particles.draw();
}

} // physics
} // clover
