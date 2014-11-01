#ifndef CLOVER_GAME_PHYSICS_HPP
#define CLOVER_GAME_PHYSICS_HPP

#include "build.hpp"

namespace clover {
namespace physics {

class Entity;
class Object;

} // physics
namespace game {

class WorldEntity;

void setOwnerWe(physics::Object& phys_obj, WorldEntity* entity);
void setOwnerWe(physics::Entity& phys_entity, WorldEntity* entity);

WorldEntity* getOwnerWe(const physics::Object& phys_obj);
WorldEntity* getOwnerWe(const physics::Entity& phys_entity);

} // game
} // clover

#endif // CLOVER_GAME_PHYSICS_HPP
