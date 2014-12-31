#include "nodeinstance_we_spatialeventdispatcher.hpp"
#include "game/world_mgr.hpp"
#include "game/worldquery.hpp"

namespace clover {
namespace nodes {

CompNode* WeSpatialEventDispatcherNodeInstance::compNode()
{
	auto n= new CompNode{};
	n->addInputSlot("event", SignalType::Event);
	n->addInputSlot("transform", SignalType::SrtTransform2);
	n->addInputSlot("radius", SignalType::Real, 1.0);

	// Event is not sent but forwarded if this is attached
	n->addOutputSlot("forward", SignalType::Event);
	return n;
}

void WeSpatialEventDispatcherNodeInstance::create(){
	transformInput= addInputSlot<SignalType::SrtTransform2>("transform");
	eventInput= addInputSlot<SignalType::Event>("event");
	radiusInput= addInputSlot<SignalType::Real>("radius");
	
	forwardOutput= addOutputSlot<SignalType::Event>("forward");
	
	eventInput->setOnReceiveCallback([&] () {
		NodeEvent event= eventInput->get();
		
		game::WESet set= game::gWorldMgr->getQuery().getEntitiesInRadius(transformInput->get().translation, radiusInput->get());
		
		event.addReceiver(set);
		
		if (forwardOutput->isAttached()){
			forwardOutput->send(event);
		}
		else {
			event.queue();
		}
	});
}

void WeSpatialEventDispatcherNodeInstance::update(){
	ensure(0);
}

} // nodes
} // clover
