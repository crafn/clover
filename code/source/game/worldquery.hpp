#ifndef CLOVER_GAME_WORLDQUERY_HPP
#define CLOVER_GAME_WORLDQUERY_HPP

#include "build.hpp"
#include "util/time.hpp"
#include "worldentity.hpp"

namespace clover {
namespace game {

class WESet;
class WeHandle;

/// @todo This shouldn't be a class
class WorldQuery {
public:
	WorldQuery();

	game::WESet getEntitiesInRadius(util::Vec2d pos, real32 radius, const util::Str8& type_name= "");
	game::WeHandle getEntityById(game::WorldEntityId id);
	game::WorldEntity* getEntityPtrById(game::WorldEntityId id);
};

} // game
} // clover

#endif // CLOVER_GAME_WORLDQUERY_HPP
