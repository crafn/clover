#include "visual_mgr.hpp"
#include "camera_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "entity_mgr.hpp"
#include "hardware/device.hpp"
#include "particle_mgr.hpp"
#include "shader_mgr.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace visual {

VisualMgr* gVisualMgr= nullptr;

VisualMgr::VisualMgr(){
	if (!gVisualMgr)
		gVisualMgr= this;

	shaderMgr= new ShaderMgr;
	entityMgr= new EntityMgr(*shaderMgr);
	cameraMgr= new CameraMgr;
	particleMgr= new ParticleMgr;
}

VisualMgr::~VisualMgr(){
	delete particleMgr; particleMgr= nullptr;
	delete cameraMgr; cameraMgr= nullptr;
	delete entityMgr; entityMgr= nullptr;
	delete shaderMgr; shaderMgr= nullptr;

	if (gVisualMgr == this)
		gVisualMgr= nullptr;
}

void VisualMgr::renderFrame(){
	{ PROFILE_("render");
		debug::gDebugDraw->update();
		particleMgr->update();
		hardware::gDevice->clearBuffers();
		entityMgr->draw();
	}
	{ PROFILE_("swapBuffers");
		hardware::gDevice->swapBuffers();
	}
}

} // visual
} // clover
