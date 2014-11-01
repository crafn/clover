#include "physics.hpp"
#include "physics/entity.hpp"
#include "physics/object.hpp"
#include "script/script_mgr.hpp"
#include "script.hpp"
#include "world_mgr.hpp"
#include "worldentity_handle.hpp"
#include "worldentity.hpp"
#include "worldgrid.hpp"

namespace clover {
namespace game {

game::WorldMgr& getWorldMgr(){
	ensure(game::gWorldMgr);
	return *game::gWorldMgr;
}

void registerToScript()
{
	auto&& script= *NONULL(script::gScriptMgr);

	script.registerObjectType<WorldEntity*>();

	script.registerObjectType<WeHandle>();
	script.registerMethod(&WeHandle::get, "get");
	script.registerMethod(&WeHandle::ref, "ref");

	script.registerObjectType<game::WeMgr*>();
	
	script.registerObjectType<game::WorldMgr*>();
	script.registerMethod(&game::WorldMgr::getWeMgr, "getWeMgr");
	script.registerMethod(&game::WorldMgr::getTime, "getTime");
	script.registerMethod(&game::WorldMgr::getDayDuration, "getDayDuration");
	script.registerMethod(&game::WorldMgr::getDayPhase, "getDayPhase");
	script.registerGlobalFunction(getWorldMgr, "getWorldMgr");
	
	script.registerGlobalFunction(game::WorldGrid::chunkCornerToWorldVec, "chunkCornerToWorldVec");
	script.registerGlobalFunction(game::WorldGrid::blockCornerToWorldVec, "blockCornerToWorldVec");
	script.registerGlobalFunction(game::WorldGrid::regionCornerToWorldVec, "regionCornerToWorldVec");
	
	script.registerGlobalFunction(game::WorldGrid::chunkCenterToWorldVec, "chunkCenterToWorldVec");
	script.registerGlobalFunction(game::WorldGrid::blockCenterToWorldVec, "blockCenterToWorldVec");
	script.registerGlobalFunction(game::WorldGrid::regionCenterToWorldVec, "regionCenterToWorldVec");
	
	script.registerGlobalProperty(game::WorldGrid::chunkWidthInBlocks, "chunkWidthInBlocks");

	script.registerGlobalFunction<void (physics::Object&, WorldEntity*)>
		(game::setOwnerWe, "setOwnerWe");
	script.registerGlobalFunction<void (physics::Entity&, WorldEntity*)>
		(game::setOwnerWe, "setOwnerWe");

	script.registerGlobalFunction<WorldEntity* (const physics::Object&)>
		(game::getOwnerWe, "getOwnerWe");
	script.registerGlobalFunction<WorldEntity* (const physics::Entity&)>
		(game::getOwnerWe, "getOwnerWe");
}

} // game
} // clover
