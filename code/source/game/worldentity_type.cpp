#include "worldentity_type.hpp"
#include "nodes/compositionnodelogicgroup.hpp"
#include "nodes/nodetype.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace game {

WeType::WeType()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(nodeGroupAttribute, "nodeGroup", "")
		, nodeGroup(nullptr){
	nodeGroupAttribute.setOnChangeCallback([&] (){
		if (getResourceState() != State::Uninit){
			print(debug::Ch::General, debug::Vb::Trivial, "WeType NodeGroup attribute changed");
			resourceUpdate(true, true); // Reload immediately to prevent two recreations in WorldEntities (= unload & reload)
		}
	});
}

void WeType::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		
		try {
			nodeGroupChangeListener.clear();
			nodeGroup= &global::g_env->resCache->getResource<nodes::CompositionNodeLogicGroup>(nodeGroupAttribute.get());
			nodeGroupChangeListener.listen(*nodeGroup, [&] (){
				// Changes of node groups are detected separately in WorldEntities, so no OnChangeCb::trigger();
				//print(debug::Ch::General, debug::Vb::Trivial, "WeType: NodeGroup changed");
				resourceUpdate(true, true);
			});
			
			auto& node= nodeGroup->getNode("WeInterface"); // Make sure that this node group can be used in game::WorldEntity
			
			if (nodeGroup->getResourceState() != State::Loaded)
				throw resources::ResourceException("WeType %s: NodeGroup not loaded: %s",
					nameAttribute.get().cStr(), nodeGroupAttribute.get().cStr());
			
			setResourceState(State::Loaded);

		}
		catch (const resources::ResourceException&){
			createErrorResource();
		}
	}
	else {
		setResourceState(State::Unloaded);
	}
}

void WeType::createErrorResource(){
	setResourceState(State::Error);
}

const nodes::CompositionNodeLogicGroup& WeType::getNodeGroup() const {
	ensure(nodeGroup);
	return *nodeGroup;
}

} // game
} // clover