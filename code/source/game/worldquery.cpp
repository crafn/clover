#include "global/env.hpp"
#include "util/math.hpp"
#include "world_mgr.hpp"
#include "worldchunk.hpp"
#include "worldchunk_mgr.hpp"
#include "worldentity_handle.hpp"
#include "worldentity_set.hpp"
#include "worldquery.hpp"

namespace clover {
namespace game {

WorldQuery::WorldQuery(){
}

game::WESet WorldQuery::getEntitiesInRadius(util::Vec2d pos, real32 radius, const util::Str8& type_name){
	game::WESet ret;

	util::Set<game::WorldChunk*> chunks= global::g_env.worldMgr->getChunkMgr().getChunksInRadius(pos, radius);

	for (auto chunk : chunks){
		util::PtrTable<game::WorldEntity>& entities= chunk->getEntityTable();

		for (uint32 e=0; e<entities.size(); e++){
			if (!entities[e])
				continue;

			if (type_name.empty() || entities[e]->getTypeName() == type_name){
				if (util::Vec2d(entities[e]->getPosition()-pos).lengthSqr() < radius*radius)
					ret.add(entities[e]);
			}
		}

	}

	return ret;
}

game::WorldEntity* WorldQuery::getEntityPtrById(game::WorldEntityId id){
	game::WorldEntity* ret= nullptr;

	auto it= game::WorldEntity::getWorldEntityMap().find(id);
	if (it != game::WorldEntity::getWorldEntityMap().end())
		ret= it->second;

	return ret;
}

game::WeHandle WorldQuery::getEntityById(game::WorldEntityId id){
	return game::WeHandle(getEntityPtrById(id));
}

} // game
} // clover
