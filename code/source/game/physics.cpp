#include "physics.hpp"
#include "physics/entity.hpp"
#include "physics/object.hpp"
#include "worldentity.hpp"

namespace clover {
namespace game {

void setOwnerWe(physics::Object& phys_obj, WorldEntity* entity)
{
	phys_obj.getProperties().set("we", WeHandle{entity});
}

void setOwnerWe(physics::Entity& phys_entity, WorldEntity* entity)
{
	phys_entity.getProperties().set("we", WeHandle{entity});
}

WorldEntity* getOwnerWe(const physics::Object& phys_obj)
{
	physics::Entity* phys_entity= phys_obj.getEntity();
	if (phys_entity)
		return getOwnerWe(*phys_entity);

	const WeHandle* h= phys_obj.getProperties().find<WeHandle>("we");
	return h ? h->get() : nullptr;
}

WorldEntity* getOwnerWe(const physics::Entity& phys_entity)
{
	const WeHandle* h= phys_entity.getProperties().find<WeHandle>("we");
	return h ? h->get() : nullptr;
}

} // game
} // clover
