#ifndef CLOVER_GAME_WORLDENTITY_TABLE_HPP
#define CLOVER_GAME_WORLDENTITY_TABLE_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/ptrtable.hpp"

namespace clover {
namespace game {

class WorldEntity;

typedef uint64 WorldEntityId;

/// @todo Remove. Ids should be managed by WeMgr
class ENGINE_API WETable : public util::PtrTable<game::WorldEntity> {
public:
	WETable();

	game::WorldEntityId assignUniqueId();
	
	game::WorldEntityId getNextUniqueId() const;
	void setNextUniqueId(game::WorldEntityId);

private:
	game::WorldEntityId nextUniqueEntityId;
};

/// @return WETable of global::g_env.worldMgr->weMgr
WETable& getWeTable();

} // game
} // clover

#endif // CLOVER_GAME_WORLDENTITY_TABLE_HPP
