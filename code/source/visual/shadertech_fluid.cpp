#include "shadertech_fluid.hpp"
#include "debug/debugdraw.hpp"
#include "physics/draw.hpp"
#include "physics/phys_mgr.hpp"
#include "resources/cache.hpp"
#include "shader.hpp"
#include "global/file.hpp"
#include "visual/camera.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace visual {

static bool useFluidDebugDraw(){
	return debug::gDebugDraw->getPhysicsDraw().getFlag(
				physics::Draw::Flag::FluidParticles);
}

FluidST::DebugDraw::DebugDraw(){
	shader.setSources(
		global::File::readText("fluids/render_particledebug.vert"),
		global::File::readText("fluids/render_particledebug.geom"),
		global::File::readText("fluids/render_particledebug.frag"));
	shader.compile<physics::FluidParticle>();
}

FluidST::ParticleDraw::ParticleDraw(){
	shader.setSources(
		global::File::readText("fluids/render_particle.vert"),
		global::File::readText("fluids/render_particle.geom"),
		global::File::readText("fluids/render_particle.frag"));
	shader.compile<physics::FluidParticle>();
}

FluidST::DistFieldDraw::DistFieldDraw(){
	shader.setSources(
		global::File::readText("fluids/render_distancefield.vert"),
		global::File::readText(""),
		global::File::readText("fluids/render_distancefield.frag"));
	shader.compile<physics::VolumVertex>();
}

FluidST::PostProcess::PostProcess(){
	shader.setSources(
		global::File::readText("fluids/render_postprocess.vert"),
		"",
		global::File::readText("fluids/render_postprocess.frag"));
	shader.compile<visual::Vertex>();
}

static constexpr SizeType debugDrawId= 0;
static constexpr SizeType particleDrawId= 1;
static constexpr SizeType distFieldDrawId= 2;

FluidST::FluidST(){
	quad= &resources::gCache->getResource<visual::TriMesh>("unitRect");
}

void FluidST::render(Camera& cam){
	physics::FluidMgr* fluid_mgr=
		physics::gPhysMgr->getFluidMgr();
	if (!fluid_mgr)
		return;

	auto& fbo= cam.getFluidFbo();

	if (!fbo.isCreated())
		return;

	bool debug_draw= useFluidDebugDraw();

	{
		Shader& shd= debug_draw ? debugDraw.shader : particleDraw.shader;

		ModelEntityDef def;
		Model model;
		model.setMesh(fluid_mgr->getParticles());
		def.setModel(model);

		if (!debug_draw){
			fbo.bind();
			hardware::gGlState->setClearColor({0.0, 0.0, 0.0, 0.0});
			hardware::gGlState->clear(GL_COLOR_BUFFER_BIT);
		}

		setCamera(cam);
		setEntity(def);
		setTransform(
			util::SrtTransform3d{	util::Vec3d(1),
									util::Quatd::identity(),
									util::Vec3d(0)});
		use(shd);

		hardware::gGlState->setBlendFunc(hardware::GlState::BlendFunc{
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});

		def.draw();
	}

	if (!debug_draw){
		// Draw distance field to fbo
		setCamera(cam);
		use(distFieldDraw.shader);

		hardware::gGlState->setBlendFunc(hardware::GlState::BlendFunc{
			GL_SRC_ALPHA, GL_ONE});
		fluid_mgr->getDistanceField().getMappedMesh().draw();
	}

	if (!debug_draw){
		// Draw from cam fbo to screen
		hardware::gGlState->setBlendFunc(hardware::GlState::BlendFunc{
				GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});
		hardware::gGlState->bindFbo(0);
		hardware::gDevice->setViewport();

		postProcess.shader.use();
		postProcess.shader.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_tex", fbo.getTextureDId(), 0);

		quad->draw();
	}
}

/// @todo Distribute to render(..) and remove
void FluidST::use(Shader& shd){
	physics::FluidMgr* fluid_mgr= physics::gPhysMgr->getFluidMgr();
	if (!fluid_mgr)
		return;

	Base::use(shd);

	if (&shd == &particleDraw.shader){
		particleDraw.shader.setUniform(
				"u_chunkSize", fluid_mgr->getChunkSize());
		particleDraw.shader.setUniform(
				"u_chunkCount", fluid_mgr->getMaxChunkCount());

		hardware::GlState::TexDId ch_info_tex= fluid_mgr->getChunkInfoTexDId();
		particleDraw.shader.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_chunkInfo", ch_info_tex, 1);

		particleDraw.shader.setUniform(
				"u_timeOffset",
				physics::gPhysMgr->getFluidTimeOffset());

		particleDraw.shader.setUniform(
				"u_hueCycle",
				util::gGameClock->getTime());

	}
	else if (&shd == &distFieldDraw.shader) {
		const visual::Framebuffer& fbo=
			fluid_mgr->getDistanceField().getFbo();

		distFieldDraw.shader.setTexture(
				hardware::GlState::TexTarget::Tex3d,
				"u_tex", fbo.getTextureDId(), 1);
	}
	else if (&shd == &debugDraw.shader){
		debugDraw.shader.setUniform(
				"u_chunkSize", fluid_mgr->getChunkSize());
		debugDraw.shader.setUniform(
				"u_chunkCount", fluid_mgr->getMaxChunkCount());

		hardware::GlState::TexDId ch_info_tex= fluid_mgr->getChunkInfoTexDId();
		debugDraw.shader.setTexture(
				hardware::GlState::TexTarget::Tex2d,
				"u_chunkInfo", ch_info_tex, 1);
	}
}

} // visual
} // clover
