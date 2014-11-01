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

	chunkSizeLoc= shader.getUniformLocation("u_chunkSize");
	chunkCountLoc= shader.getUniformLocation("u_chunkCount");
	chunkInfoSamplerLoc= shader.getUniformLocation("u_chunkInfo");
}

FluidST::ParticleDraw::ParticleDraw(){
	shader.setSources(
		global::File::readText("fluids/render_particle.vert"),
		global::File::readText("fluids/render_particle.geom"),
		global::File::readText("fluids/render_particle.frag"));
	shader.compile<physics::FluidParticle>();

	chunkSizeLoc= shader.getUniformLocation("u_chunkSize");
	chunkCountLoc= shader.getUniformLocation("u_chunkCount");
	chunkInfoSamplerLoc= shader.getUniformLocation("u_chunkInfo");
	timeOffsetLoc= shader.getUniformLocation("u_timeOffset");
	hueCycleLoc= shader.getUniformLocation("u_hueCycle");
}

FluidST::DistFieldDraw::DistFieldDraw(){
	shader.setSources(
		global::File::readText("fluids/render_distancefield.vert"),
		global::File::readText(""),
		global::File::readText("fluids/render_distancefield.frag"));
	shader.compile<physics::VolumVertex>();

	texLoc= shader.getUniformLocation("u_tex");
}

FluidST::PostProcess::PostProcess(){
	shader.setSources(
		global::File::readText("fluids/render_postprocess.vert"),
		"",
		global::File::readText("fluids/render_postprocess.frag"));
	shader.compile<visual::Vertex>();

	texLoc= shader.getUniformLocation("u_tex");
}

static constexpr SizeType debugDrawId= 0;
static constexpr SizeType particleDrawId= 1;
static constexpr SizeType distFieldDrawId= 2;

FluidST::FluidST(){
	shaders.pushBack(&debugDraw.shader);
	locateUniforms(debugDrawId);

	shaders.pushBack(&particleDraw.shader);
	locateUniforms(particleDrawId);

	shaders.pushBack(&distFieldDraw.shader);
	locateUniforms(distFieldDrawId);

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
		// Draw particles to fbo with some ugly code
		// (whole shadertech system should be refactored)

		if (!debug_draw){
			shaderIndex= particleDrawId;
		}
		else {
			shaderIndex= debugDrawId;
		}

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
		setTransformation(util::Vec2d(0.0), util::Quatd::identity());
		setScale(util::Vec2d(1.0));
		use();

		hardware::gGlState->setBlendFunc(hardware::GlState::BlendFunc{
			GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA});

		def.draw();
	}

	if (!debug_draw){
		// Draw distance field to fbo
		shaderIndex= distFieldDrawId;
		setCamera(cam);
		use();

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
				postProcess.texLoc,
				fbo.getTextureDId(),
				0);

		quad->draw();
	}
}

void FluidST::use(){
	physics::FluidMgr* fluid_mgr= physics::gPhysMgr->getFluidMgr();
	if (!fluid_mgr)
		return;

	Base::use();

	if (shaderIndex == particleDrawId){
		particleDraw.shader.setUniform(
				particleDraw.chunkSizeLoc, fluid_mgr->getChunkSize());
		particleDraw.shader.setUniform(
				particleDraw.chunkCountLoc, fluid_mgr->getMaxChunkCount());

		hardware::GlState::TexDId ch_info_tex= fluid_mgr->getChunkInfoTexDId();
		particleDraw.shader.setTexture(
				particleDraw.chunkInfoSamplerLoc, ch_info_tex, 1);

		particleDraw.shader.setUniform(
				particleDraw.timeOffsetLoc,
				physics::gPhysMgr->getFluidTimeOffset());

		particleDraw.shader.setUniform(
				particleDraw.hueCycleLoc,
				util::gGameClock->getTime());

	}
	else if (shaderIndex == distFieldDrawId) {
		const visual::Framebuffer& fbo=
			fluid_mgr->getDistanceField().getFbo();

		distFieldDraw.shader.setTexture(
				hardware::GlState::TexTarget::Tex3d,
				distFieldDraw.texLoc,
				fbo.getTextureDId(),
				1);
	}
	else if (shaderIndex == debugDrawId){
		debugDraw.shader.setUniform(
				debugDraw.chunkSizeLoc, fluid_mgr->getChunkSize());
		debugDraw.shader.setUniform(
				debugDraw.chunkCountLoc, fluid_mgr->getMaxChunkCount());

		hardware::GlState::TexDId ch_info_tex= fluid_mgr->getChunkInfoTexDId();
		debugDraw.shader.setTexture(
				debugDraw.chunkInfoSamplerLoc, ch_info_tex, 1);
	}
}

} // visual
} // clover