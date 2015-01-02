#include "camera_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "entity_mgr.hpp"
#include "global/cfg_mgr.hpp"
#include "global/env.hpp"
#include "hardware/device.hpp"
#include "particle_mgr.hpp"
#include "resources/cache.hpp"
#include "shader_mgr.hpp"
#include "util/profiling.hpp"
#include "visual_mgr.hpp"

// Resources
#include "shadertemplate.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "entity_def.hpp"
#include "entity_def_model.hpp"
#include "entity_def_light.hpp"
#include "entity_def_compound.hpp"

namespace clover {
namespace visual {

VisualMgr* gVisualMgr= nullptr;

VisualMgr::VisualMgr()
	: modelDefMem(sizeof(ModelEntityDef)*
			global::gCfgMgr->get<SizeType>(
				"visual::maxModelDefCount"),
			"visual::modelDefMem")
	, modelLogicMem(sizeof(ModelEntityLogic)*
			global::gCfgMgr->get<SizeType>(
				"visual::maxModelEntityCount"),
			"visual::modelEntityMem")
{
	if (!gVisualMgr)
		gVisualMgr= this;

	ModelEntityDef::setPoolMem(&modelDefMem);
	ModelEntityLogic::setPoolMem(&modelLogicMem);

	global::g_env.resCache->preLoad<ShaderTemplate>();
	global::g_env.resCache->preLoad<Texture>();
	global::g_env.resCache->preLoad<Material>();
	global::g_env.resCache->preLoad<TriMesh>();
	global::g_env.resCache->preLoad<Model>();
	global::g_env.resCache->preLoad<EntityDef>();
	global::g_env.resCache->preLoad<ModelEntityDef>();
	global::g_env.resCache->preLoad<LightEntityDef>();
	global::g_env.resCache->preLoad<CompoundEntityDef>();

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

	global::g_env.resCache->unload<CompoundEntityDef>();
	global::g_env.resCache->unload<LightEntityDef>();
	global::g_env.resCache->unload<ModelEntityDef>();
	global::g_env.resCache->unload<EntityDef>();
	global::g_env.resCache->unload<Model>();
	global::g_env.resCache->unload<TriMesh>();
	global::g_env.resCache->unload<Material>();
	global::g_env.resCache->unload<Texture>();
	global::g_env.resCache->unload<ShaderTemplate>();

	ModelEntityLogic::setPoolMem(nullptr);
	ModelEntityDef::setPoolMem(nullptr);

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
