#include "ec_we_tools.hpp"
#include "debug/debugdraw.hpp"
#include "game/worldentity.hpp"
#include "game/worldentity_set.hpp"
#include "game/world_mgr.hpp"
#include "hardware/keyboard.hpp"
#include "physics/phys_mgr.hpp"
#include "physics/world.hpp"
#include "ui/userinput.hpp"
#include "visual/camera_mgr.hpp"
#include "visual/visual_mgr.hpp"

namespace clover {
namespace game { namespace editor {

WeToolsEc::WeToolsEc(){
}

void WeToolsEc::spawn(uint32 name_id){
	game::WeHandle h= game::gWorldMgr->getWeMgr().createEntity(	
					getWeNames()[name_id],
					visual::gVisualMgr->getCameraMgr().getSelectedCamera().getPosition() + util::Vec2d{0,3});
}

void WeToolsEc::setDebugDrawActive(bool b){
	debug::gDebugDraw->setEnabled(debug::DebugDraw::DrawFlag::Common, b);
}

bool WeToolsEc::isDebugDrawActive() const {
	return debug::gDebugDraw->isEnabled(debug::DebugDraw::DrawFlag::Common);
}

void WeToolsEc::setChunksLocked(bool b){
	game::gWorldMgr->setChunksLocked(b);
}
bool WeToolsEc::isChunksLocked() const {
	return game::gWorldMgr->isChunksLocked();
}

util::DynArray<util::Str8> WeToolsEc::getWeNames() const {
	util::DynArray<util::Str8> ret;

	for (auto& m : resources::gCache->getSubCache<game::WeType>().getResources()){
		ret.pushBack(m->getName());
	}
	
	return ret;
}

void WeToolsEc::eraseTerrain(util::Vec2d pos){
	physics::gPhysMgr->getWorld().applyRadialStressField(pos, 20.0, 1.0);
}

void WeToolsEc::deleteWe(util::Vec2d pos){
	game::WESet w= game::gWorldMgr->getQuery().getEntitiesInRadius(pos,0.4);
	for (auto it= w.begin(); it!= w.end(); ++it){
		if ((*it)){
			(*it)->setRemoveFlag();
		}
	}
}

}} // game::editor
} // clover