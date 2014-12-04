#include "game/world_mgr.hpp"
#include "game/worldentity.hpp"
#include "nodeinstance_we_edgespawner.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

void WeEdgeSpawnerNodeInstance::create()
{
	edgeEntityIn= addInputSlot<SignalType::String>("edgeEntity");
	weIn= addInputSlot<SignalType::WeHandle>("we");
	spawnIn= addInputSlot<SignalType::Trigger>("spawn");

	edgeEntityIn->setOnReceiveCallback([&] ()
	{
		edgeType=
			&resources::gCache->getResource<game::WeType>(edgeEntityIn->get());
	});

	spawnIn->setOnReceiveCallback([&] ()
	{
		spawnerType= &NONULL(weIn->get().get())->getType();
		game::gWorldMgr->onEdgeSpawnTrigger(*this);
	});

	setUpdateNeeded(false);
}

void WeEdgeSpawnerNodeInstance::update()
{ }

} // node
} // clover
