#include "worldentity_handle.hpp"
#include "worldentity_handleconnection.hpp"
#include "worldentity.hpp"

namespace clover {
namespace game {

util::PtrTable<WeHandle> WeHandle::lostTable;
bool WeHandle::newLostHandle= false;


WeHandle::WeHandle(const game::WorldEntity* e)
		: entityId(0)
		, tableIndex(0)
		, lostIndex(-1)
		, strictConnection(0){
	if (e){
			entityId= e->uniqueId;
			tableIndex= e->tableIndex;
	}
}

WeHandle::~WeHandle(){
	if (lostIndex >= 0){
		lostTable[lostIndex]= 0;
		//print(debug::Ch::General, debug::Vb::Trivial, "Lost WEHandle destroyed: %i, %i", lostIndex, entityId);
		//ensure(0);
	}
	
	if (strictConnection){
		//print(debug::Ch::General, debug::Vb::Trivial, "WEHandle destroyed: %i", (bool)strictConnection);
		delete strictConnection;
		strictConnection=0;
	}
}

WeHandle::WeHandle(const WeHandle& h):
				entityId(h.entityId),
				tableIndex(h.tableIndex),
				lostIndex(-1),
				strictConnection(0){ // strictConnection not copied
	
	updateLostness();
}

WeHandle::WeHandle(WeHandle&& h)
		: entityId(h.entityId)
		, tableIndex(h.tableIndex)
		, lostIndex(h.lostIndex)
		, strictConnection(h.strictConnection){
	if (strictConnection)
		strictConnection->resetHandle(*this);
	h.entityId= 0;
	h.tableIndex= 0;
	h.lostIndex= -1;
	h.strictConnection= 0;
}

WeHandle& WeHandle::operator=(const WeHandle& h){
	entityId= h.entityId;
	tableIndex= h.tableIndex;
	return *this;
}

bool WeHandle::isGood() const {
	if (entityId == 0)
		return false;
	
	game::WorldEntity* e= gWETable[tableIndex];
	if (e != 0 && e->uniqueId == entityId)
		return true;
	
	return false;
}

bool WeHandle::isLost() const {
	if (entityId != 0 && !isGood()) return true;
	return false;
}

void WeHandle::updateLostness(){
	if (isLost() && lostIndex < 0){
		lostIndex= lostTable.findFreeIndex();
		lostTable[lostIndex]= this;
		newLostHandle= true;
		
		//print(debug::Ch::General, debug::Vb::Trivial, "Lost WEHandle added, %i, %i", lostIndex, entityId);
	}
	else if (!isLost() && lostIndex >= 0){
		// Entity found
		//print(debug::Ch::General, debug::Vb::Trivial, "Lost WEHandle found, %i", lostIndex);
		lostTable[lostIndex]= 0;
		lostIndex= -1;
	}
}

void WeHandle::setId(game::WorldEntityId id){
	entityId= id;
	updateLostness();
}

void WeHandle::reset(){
	entityId= 0;
	updateLostness();
}

bool WeHandle::isAssigned(){
	if (isGood()) return true;
	if (isLost()) return true;
	return false;
}

game::WorldEntity *WeHandle::get() const {
	if (!isGood()) return 0;
	return gWETable[tableIndex];
}

game::WorldEntity& WeHandle::ref() const {
	ensure(get());
	return *get();
}

WeHandle::operator bool() {
	updateLostness();
	return isGood();
}

WeHandle::operator bool() const {
	return isGood();
}

game::WorldEntity* WeHandle::operator->() const {
	ensure_msg(isGood(), "WEHandle::operator->(): invalid handle for id: %lld", entityId)
	return get();
}

void WeHandle::fixLostHandles(game::WorldEntity* w){
	game::WorldEntityId id= w->uniqueId;
	for (uint32 i=0; i<lostTable.size(); ++i){
		if (lostTable[i] == 0 || lostTable[i]->isGood()) continue;
		
		// Löyty rikkinäinen handle
		if (lostTable[i]->entityId == id){
			lostTable[i]->tableIndex= w->tableIndex;
			lostTable[i]->updateLostness();
			
			
		}
	}
}

void WeHandle::fixLostHandles(){
	newLostHandle= false;
	for (uint32 i=0; i<lostTable.size(); ++i){
		if (lostTable[i] == 0) continue;
		WeHandle& h= *lostTable[i];

		auto it= game::WorldEntity::getWorldEntityMap().find(h.entityId);
		if (it != game::WorldEntity::getWorldEntityMap().end()){
			// Entity found
			
			h.tableIndex= it->second->tableIndex;
			h.updateLostness();

			ensure(!h.isLost() || it->second->getId() == 0);
		}
	}
}

uint32 WeHandle::countLostHandles(){
	uint32 count= 0;
	for (uint32 i= 0; i<lostTable.size(); ++i){
		if (lostTable[i] != 0) ++count;
	}
	return count;
}

void WeHandle::setStrict(game::WorldEntity& owner){
	//print(debug::Ch::WE, debug::Vb::Trivial, "setStrict: %i", owner.getId());
	ensure(!strictConnection);
	strictConnection= new StrictHandleConnection(owner, *this);
}

} // game
} // clvoer