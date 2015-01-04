#include "global/exception.hpp"
#include "world_mgr.hpp"
#include "worldentity_table.hpp"
#include "worldentity_mgr.hpp"

namespace clover {
namespace game {

WETable& getWeTable()
{ return global::g_env->worldMgr->getWeMgr().getWeTable(); }

WETable::WETable():util::PtrTable<game::WorldEntity>(100000){
	nextUniqueEntityId= 1;
}

game::WorldEntityId WETable::assignUniqueId(){
	nextUniqueEntityId++;
	return nextUniqueEntityId-1;
}

game::WorldEntityId WETable::getNextUniqueId() const {
	return nextUniqueEntityId;
}

void WETable::setNextUniqueId(game::WorldEntityId id){
	nextUniqueEntityId= id;
}

} // game
} // clover
