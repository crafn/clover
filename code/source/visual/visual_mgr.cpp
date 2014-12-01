#include "visual_mgr.hpp"
#include "camera_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "entity_mgr.hpp"
#include "global/cfg_mgr.hpp"
#include "hardware/device.hpp"
#include "particle_mgr.hpp"
#include "resources/cache.hpp"
#include "shader_mgr.hpp"
#include "util/profiling.hpp"

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

	resources::gCache->preLoad<ShaderTemplate>();
	resources::gCache->preLoad<Texture>();
	resources::gCache->preLoad<Material>();
	resources::gCache->preLoad<TriMesh>();
	resources::gCache->preLoad<Model>();
	resources::gCache->preLoad<EntityDef>();
	resources::gCache->preLoad<ModelEntityDef>();
	resources::gCache->preLoad<LightEntityDef>();
	resources::gCache->preLoad<CompoundEntityDef>();

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

	resources::gCache->unload<CompoundEntityDef>();
	resources::gCache->unload<LightEntityDef>();
	resources::gCache->unload<ModelEntityDef>();
	resources::gCache->unload<EntityDef>();
	resources::gCache->unload<Model>();
	resources::gCache->unload<TriMesh>();
	resources::gCache->unload<Material>();
	resources::gCache->unload<Texture>();
	resources::gCache->unload<ShaderTemplate>();

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
