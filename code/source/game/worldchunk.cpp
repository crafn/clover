#include "worldchunk.hpp"
#include "game/world_mgr.hpp" // Only for callbacks
#include "game/worldentity_mgr.hpp"
#include "game/worldgrid.hpp"
#include "global/env.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace game {

constexpr uint16 WorldChunk::chunkStartTag;
constexpr uint16 WorldChunk::entityStartTag;

util::DynArray<WorldChunk*> WorldChunk::nearestSorted(ChunkVec target, ChunkSet chunks){
	util::MultiMap<double, WorldChunk*> chunks_by_dist;
	
	for (auto& chunk : chunks){
		double dist_sqr= (chunk->getPosition() - target).lengthSqr();
		chunks_by_dist.insert(std::make_pair(dist_sqr, std::move(chunk)));
	}
	
	util::DynArray<WorldChunk*> sorted;
	for (auto& pair : chunks_by_dist){
		sorted.pushBack(std::move(pair.second));
	}
	ensure(sorted.size() == chunks.size());
	return sorted;
}

util::DynArray<WorldChunk::ChunkSet> WorldChunk::nearestSorted(ChunkVec target, ClusterSet clusters){
	util::MultiMap<double, ChunkSet> clusters_by_dist;
	
	for (auto& cluster : clusters){
		double closest_sqr= -1;
		for (auto& chunk : cluster){
			double dist_sqr= (chunk->getPosition() - target).lengthSqr();
			if (dist_sqr < closest_sqr || closest_sqr < 0){
				closest_sqr= dist_sqr;
			}
			clusters_by_dist.insert(std::make_pair(dist_sqr, std::move(cluster)));
		}
	}
	
	util::DynArray<ChunkSet> sorted;
	for (auto& pair : clusters_by_dist){
		sorted.pushBack(std::move(pair.second));
	}
	ensure(sorted.size() == clusters.size());
	return sorted;
}

WorldChunk::WorldChunk(ChunkVec pos)
		: state(State::None){
	for (int32 i=0; i<4; i++)
		sides[i]= 0;

	entities.resize(1024);

	setPosition(pos);
}

WorldChunk::~WorldChunk(){
	for(uint32 i=0; i<entities.size(); ++i){
		if (entities[i] == nullptr)
			continue;
		
		entities[i]->setRemoveFlag();
		entities[i]->setInChunk(nullptr);
	}
	
	// Poistetaan vierekkäisten chunkkien naapureista
	for (uint32 i= 0; i < 4; ++i){
		if (sides[i]) sides[i]->setSide((Side)((i+2)%4), nullptr);
		sides[i]= nullptr;
	}
}

void WorldChunk::setState(State s){
	if (s == state)
		return;

	auto prev= state;

	// State::Serializing can be changed only to Destroying
	ensure(	state != State::Serializing ||
			(state == State::Serializing && s == State::Destroying));

	state= s;

	for (uint32 i=0; i<entities.size(); i++){
		if (entities[i] == nullptr)
			continue;

		if (!entities[i]->isSpawned()){
			entities[i]->allowSpawning(state == State::Active);
		}

		entities[i]->setActive(state == State::Active);
	}

	global::g_env->worldMgr->onChunkStateChange(*this, static_cast<int32>(prev));
}

void WorldChunk::setPosition(ChunkVec pos){
	position= pos;
}

util::Vec2i WorldChunk::getPosition() const {
	return position;
}

void WorldChunk::setSide(WorldChunk& chunk){

	if (chunk.getPosition().x == position.x + 1 &&
		chunk.getPosition().y == position.y){
		sides[Right]= &chunk;

		if (chunk.getSide(Left) != this)
			chunk.setSide(*this);

		return;
	}
	else if (	chunk.getPosition().x == position.x - 1 &&
				chunk.getPosition().y == position.y){
		sides[Left]= &chunk;

		if (chunk.getSide(Right) != this)
			chunk.setSide(*this);

		return;
	}
	else if (chunk.getPosition().x == position.x &&
			chunk.getPosition().y == position.y + 1){
		sides[Up]= &chunk;

		if (chunk.getSide(Down) != this)
			chunk.setSide(*this);

		return;
	}
	else if (	chunk.getPosition().x == position.x &&
				chunk.getPosition().y == position.y - 1){
		sides[Down]= &chunk;

		if (chunk.getSide(Up) != this)
			chunk.setSide(*this);

		return;
	}

	throw(global::Exception("WorldChunk::setSide(..): failed"));

}

WorldChunk* WorldChunk::getSide(Side s){
	return sides[s];
}

void WorldChunk::addEntity(game::WorldEntity& obj){
	
	for (auto m : entities){
		ensure_msg(m != &obj, "Entity already in chunk:, %s, %i (%i, %i)", obj.getTypeName().cStr(), obj.getId(), position.x, position.y);
	}
	
	entities.findFree()= &obj;
	obj.setInChunk(this);
	
	if (obj.isActive() != (state == State::Active)) 
		obj.setActive(state == State::Active);

	if (!obj.isSpawned()){
		obj.allowSpawning(state == State::Active);
	}

	game::GridPoint p;
	p.setWorldVec(obj.getPosition());
	
	util::Vec2i chunk_pos= p.getChunkVec();
	
	ensure_msg(chunk_pos  == position,
		"Positions didn't match: %i, %i == %i, %i, entity: %s, (%f, %f)", 
			chunk_pos.x, chunk_pos.y, 
			position.x, position.y, 
			obj.getTypeName().cStr(),
			obj.getPosition().x, obj.getPosition().y);
	
}

void WorldChunk::removeEntity(game::WorldEntity& obj){
	uint32 found=0;
	for (uint32 i=0; i<entities.size(); i++){
		if (&obj == entities[i]){
			removeEntity(i);
			++found;
		}
	}
	
	ensure_msg(found == 1, "%i found (1 is correct)", found);
}

SizeType WorldChunk::getEntityCount() const {
	return entities.getUsedCount();
}

void WorldChunk::clearDependencies(){
	dependencyMap.clear();
}

void WorldChunk::addDependency(WorldChunk& c){
	dependencyMap[c.getPosition()]= &c;
}

void WorldChunk::removeDependency(WorldChunk& c){
	auto it= dependencyMap.find(c.getPosition());
	if (it != dependencyMap.end())
		dependencyMap.erase(it);
}

WorldChunk::ChunkSet WorldChunk::getCluster() const {
	ChunkSet cluster= {const_cast<WorldChunk*>(this)};
	ChunkSet search_nodes= {const_cast<WorldChunk*>(this)};

	bool new_found= true;
	while (new_found){
		new_found= false;
		ChunkSet new_nodes;

		for (auto &cur_node : search_nodes){
			for (auto it= cur_node->depBegin(); it!= cur_node->depEnd(); ++it){
				if ( !cluster.count(it->second) && !new_nodes.count(it->second)){
					new_nodes.insert(it->second);
					cluster.insert(it->second);
					new_found= true;
				}
			}
		}
		search_nodes.clear();
		search_nodes= new_nodes;
	}
	
	return cluster;
}

SizeType WorldChunk::updateDestroying(SizeType max_count){
	ensure(state == State::Destroying);
	SizeType removed= 0;
	
	while (removed < max_count){
		// Entity was in a chunk which was removed
		// Prevent choppiness by not removing all of these types of entities in one frame

		int32 i= entities.findUsedIndex();
		if (i == -1) return removed;
		
		entities[i]->setRemoveFlag();
		removeEntity(i);
		
		++removed;
	
	}
	
	return removed;		
}


util::DynArray<uint8> WorldChunk::getEntityData() const {
	ensure(getState() == State::Serializing);
	return serializeEntities(util::asArrayView(entities));
}

util::DynArray<uint8> WorldChunk::serializeEntities(
		util::ArrayView<const WorldEntity* const> entities){
	util::RawPack entityPack;
	for (uint32 i=0; i < entities.size(); ++i){
		if (!entities[i])
			continue;

		WEPack pack;
		entities[i]->saveFieldSerialize(pack);

		ensure(pack.getVarPack().size() != 0);

		entityPack	<< entityStartTag
					<< pack.getId()
					<< pack.getVarBlockSize()
					<< pack.getVarPack();
	}
	return entityPack.getData();
}

void WorldChunk::createNextEntity(util::RawPack& data_pack, WorldChunk* chunk){
	uint16 tag= 0;
	// Entity info
	uint64 id;
	uint32 data_size;
	{ PROFILE();
		data_pack >> tag;
		ensure(tag == game::WorldChunk::entityStartTag);
		data_pack >> id >> data_size;
	}

	WEPack pack;
	{ PROFILE();
		pack.reset(id);
		pack.getVarPack().addData(&data_pack[data_pack.tellg()], data_size);

		data_pack.seekg(data_pack.tellg() + data_size);
	}

	{ PROFILE();
		game::WeHandle h= global::g_env->worldMgr->getWeMgr().minimalCreateEntity(pack.getId());
		// Spawning is allowed when every entity in chunk cluster
		// have been created
		h->allowSpawning(chunk == nullptr);
		h->saveFieldDeserialize(pack);
		if (chunk)	
			chunk->addEntity(*h.get());
	}
}

void WorldChunk::removeEntity(SizeType index){
	game::WorldEntity& obj= *NONULL(entities[index]);

	entities.remove(index);
	obj.setInChunk(0);
}

} // game
} // clover
