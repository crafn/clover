#include "worldentity_mgr.hpp"
#include "global/env.hpp"
#include "global/event.hpp"
#include "util/profiling.hpp"
#include "worldentity_table.hpp"
#include "worldchunk.hpp"
#include "worldchunk_mgr.hpp"
#include "worldentity_handle.hpp"
#include "worldquery.hpp"
#include "world_mgr.hpp"

namespace clover {
namespace game {

WeMgr::WeMgr(){
}

WeMgr::~WeMgr(){
	removeAll();
}

void WeMgr::update(){
	PROFILE_("worldEntity");

	util::DynArray<game::WorldEntity*> update_needing;
	nodes::UpdateLine line;

	{ PROFILE();
		// Find entities and nodes which need an update
		for(SizeType i= 0; i < gWETable.size(); ++i){
			game::WorldEntity* we= gWETable[i];
			if (!we || !we->spawned || we->remove)
				continue;

			if (!we->isUpdateNeeded())
				continue;

			{ PROFILE();
				update_needing.pushBack(we);
				line.append(std::move(we->getUpdateLine()));
			}
		}
	}

	// Allow priorization of certain nodes
	line.sort();

	// Update internals of entities
	line.run();

	// Update outer shells of entities
	shallowUpdate(update_needing);
}

void WeMgr::shallowUpdate(const util::DynArray<game::WorldEntity*> update_needing){
	PROFILE();

	for (game::WorldEntity* we : update_needing){
		if (we->isGlobal()){
			// Global entities don't care about chunks
			we->shallowUpdate();
		}
		else {
			game::WorldChunk* before= we->getInChunk();
			ChunkVec before_chunkpos= before->getPosition();
			WorldVec beforepos= we->getPosition();
			ChunkVec beforepos_chunk= game::WorldGrid::worldToChunkVec(beforepos);

			bool beforechunk_was_correct= true;
			if (	!before ||
					beforepos_chunk != before_chunkpos)
				beforechunk_was_correct= false;

			we->shallowUpdate();

			WorldVec afterpos= we->getPosition();
			ChunkVec afterpos_chunk= game::WorldGrid::worldToChunkVec(afterpos);

			if (	!beforechunk_was_correct ||
					before_chunkpos != afterpos_chunk){

				game::WorldChunk* after= global::g_env.worldMgr->getChunkMgr().getChunk(
						game::GridPoint::Chunk(afterpos_chunk));

				bool afterchunk_is_correct= true;

				if (!after || after->getState() != game::WorldChunk::State::Active){
					// Entity has rolled in non-active space
					we->setPosition(beforepos);
					afterchunk_is_correct= false;
				}

				if (	afterchunk_is_correct &&
						(after != before || !beforechunk_was_correct)){
					// Switched chunk

					if (before)
						before->removeEntity(*we);

					after->addEntity(*we);
				}

				// Entity shouldn't lose chunk
				ensure_msg( !(before && !we->getInChunk()), "Fatal fail, chunks messed up");
			}
		}
	}
}

void WeMgr::spawnNewEntities(){
	PROFILE_("worldEntity");
	bool something_to_spawn= false;
	util::DynArray<game::WorldEntity*>::Iter it;

	for (it= gWETable.begin(); it!= gWETable.end(); it++){
		if ((*it) && !(*it)->spawned && (*it)->spawningAllowed && !(*it)->remove){
			something_to_spawn= true;
			break;
		}
	}

	// Try fix handles
	if ( game::WeHandle::isNewLostHandles() || something_to_spawn )
		game::WeHandle::fixLostHandles();

		if (something_to_spawn){
		for (it= gWETable.begin(); it!= gWETable.end(); it++){
			if ((*it) && !(*it)->spawned && (*it)->spawningAllowed && !(*it)->remove){
				(*it)->spawn();
				if (!(*it)->hasSpawned())
					throw global::Exception("WEMgr::spawnNewEntities(): spawn() called, but entity %s didn't spawn", (*it)->getTypeName().cStr());
			}
		}
	}
}

void WeMgr::removeFlagged(){
	PROFILE_("worldEntity");

	util::DynArray<game::WorldEntity*>::Iter it;
	for(it= gWETable.begin(); it != gWETable.end(); it++){
		game::WorldEntity* entity= *it;
		if (!entity) continue;

		if (entity->remove){
			if (!entity->isGlobal()){
				game::WorldChunk* chunk= (*it)->inChunk;
				if (chunk)
					chunk->removeEntity(*entity);
			}
			delete entity;
		}
	}
}

SizeType WeMgr::getEntityCount() const {
	PROFILE_("worldEntity");

	util::DynArray<game::WorldEntity*>::Iter it;
	int32 count=0;
	for(it=gWETable.begin(); it!=gWETable.end(); it++){
		if ((*it) == 0)
			continue;
		count ++;
	}
	return count;
}

util::DynArray<WorldEntity*> WeMgr::getGlobalEntities() const {
	PROFILE_("worldEntity");

	util::DynArray<WorldEntity*> globals;
	for (SizeType i= 0; i < gWETable.size(); ++i){
		WorldEntity* we= gWETable[i];
		if (!we || !we->isGlobal())
			continue;
		globals.pushBack(we);
	}
	return globals;
}

void WeMgr::removeAll(){
	PROFILE_("worldEntity");

	global::g_env.worldMgr->getChunkMgr().removeAll();

	util::DynArray<game::WorldEntity*>::Iter it;
	for(it=gWETable.begin(); it!=gWETable.end(); it++){
		if ((*it) == 0)
			continue;
	
		// We removes itself from the table
		delete (*it);
	}
}

void WeMgr::onEvent(global::Event& e){
	PROFILE_("worldEntity");

	switch(e.getType()){
		default: break;
	}
}

game::WeHandle WeMgr::minimalCreateEntity(game::WorldEntityId id){
	PROFILE_("worldEntity");
	
	game::WorldEntity* we= new game::WorldEntity();
	if(id != 0)
		we->assignId(id);
	
	return game::WeHandle(we);
}

game::WeHandle WeMgr::createEntity(const util::Str8& type_name){
	PROFILE_("worldEntity");

	game::WorldEntity* we= new game::WorldEntity();
	we->setType(type_name);
	ensure_msg(we->isGlobal(), "Non-global entity created without position");
	return game::WeHandle(we);
}

game::WeHandle WeMgr::createEntity(const util::Str8& type_name, util::Vec2d position, game::WorldChunk* c){
	PROFILE_("worldEntity");

	game::WorldEntity* we= new game::WorldEntity();
	we->setPosition(position);

	if (c){
		ensure(c->getPosition() == game::WorldGrid::worldToChunkVec(position));
		c->addEntity(*we);
	}
	else {
		game::WorldChunk* ch= global::g_env.worldMgr->getChunkMgr().getChunk(game::GridPoint::World(position));
		
		if (ch)
			ch->addEntity(*we);
		else
			print(debug::Ch::WE, debug::Vb::Moderate, "Entity %s (pos: %f, %f) created outside WorldChunks (id: %lu)",
											type_name.cStr(),
											we->getPosition().x,
											we->getPosition().y,
											(unsigned long)we->getId());
	}
	
	we->setType(type_name);
	return game::WeHandle(we);
}

void WeMgr::corruptionCheck() const {
	// Check that every id is unique and within allowed boundaries
	util::Map<game::WorldEntityId, int> id_map;
	for (auto& entity : gWETable){
		if (!entity) continue;
		
		game::WorldEntityId id= entity->getId();
		if (id == 0 && entity->getRemoveFlag()) continue;
		
		release_ensure(id > 0);
		release_ensure(id < gWETable.getNextUniqueId());
		
		auto it= id_map.find(id);
		release_ensure_msg(it == id_map.end(), "Duplicate game::WorldEntity id: %lld, name: %s", id, entity->getTypeName().cStr());
		id_map.insert(std::make_pair(id, 1));
		
	}
}

} // game
} // clover
