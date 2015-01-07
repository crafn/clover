#include "ec_we_tools.hpp"
#include "debug/draw.hpp"
#include "game/world_mgr.hpp"
#include "game/worldentity.hpp"
#include "game/worldentity_mgr.hpp"
#include "game/worldentity_set.hpp"
#include "game/worldquery.hpp"
#include "global/env.hpp"
#include "hardware/keyboard.hpp"
#include "physics/phys_mgr.hpp"
#include "physics/world.hpp"
#include "ui/userinput.hpp"
#include "visual/camera.hpp"
#include "visual/camera_mgr.hpp"
#include "visual/visual_mgr.hpp"

namespace clover {
namespace game { namespace editor {

WeToolsEc::WeToolsEc(){
}

void WeToolsEc::spawn(uint32 name_id){
	game::WeHandle h= global::g_env->worldMgr->getWeMgr().createEntity(	
					getWeNames()[name_id],
					global::g_env->visualMgr->getCameraMgr().getSelectedCamera().getPosition() + util::Vec2d{0,3});
}

void WeToolsEc::setDebugDrawActive(bool b){
	global::g_env->debugDraw->setEnabled(debug::Draw::DrawFlag::Common, b);
}

bool WeToolsEc::isDebugDrawActive() const {
	return global::g_env->debugDraw->isEnabled(debug::Draw::DrawFlag::Common);
}

void WeToolsEc::setChunksLocked(bool b){
	global::g_env->worldMgr->setChunksLocked(b);
}
bool WeToolsEc::isChunksLocked() const {
	return global::g_env->worldMgr->isChunksLocked();
}

util::DynArray<util::Str8> WeToolsEc::getWeNames() const {
	util::DynArray<util::Str8> ret;

	for (auto& m : global::g_env->resCache->getSubCache<game::WeType>().getResources()){
		ret.pushBack(m->getName());
	}
	
	return ret;
}

void WeToolsEc::eraseTerrain(util::Vec2d pos){
	global::g_env->physMgr->getWorld().applyRadialStressField(pos, 20.0, 1.0);
}

void WeToolsEc::deleteWe(util::Vec2d pos){
	game::WESet w= global::g_env->worldMgr->getQuery().getEntitiesInRadius(pos,0.4);
	for (auto it= w.begin(); it!= w.end(); ++it){
		if ((*it)){
			(*it)->setRemoveFlag();
		}
	}
}

}} // game::editor
} // clover
