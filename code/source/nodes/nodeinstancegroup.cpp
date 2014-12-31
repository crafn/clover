#include "nodeinstancegroup.hpp"
#include "compositionnodelogic.hpp"
#include "compositionnodelogicgroup.hpp"
#include "resources/cache.hpp"
#include "nodetype.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace nodes {

NodeInstanceGroup::NodeInstanceGroup(const CompositionNodeLogicGroup& group){
	create(group);
}

NodeInstanceGroup::~NodeInstanceGroup(){
	//print(debug::Ch::General, debug::Vb::Trivial, "Destroying instance group: %p", this);
	compGroupListener.clear();
}

void NodeInstanceGroup::create(const CompositionNodeLogicGroup& group){
	PROFILE();
	compositionGroup= &group;

	while (!nodes.empty()){
		nodes.erase(nodes.begin());
	}
	
	compGroupListener.clear();
	compGroupListener.listen(group, [&] () {
		print(debug::Ch::Dev, debug::Vb::Trivial, "NodeInstanceGroup recreation: %s, %p", group.getName().cStr(), this);
		create(group);
	});
	
	// Try again when group is loaded
	if (group.getResourceState() != resources::Resource::State::Loaded)
		return;
	
	/// @todo Cache result and make a deep copy of it when instantiating
	util::Map<const CompositionNodeLogic*, NodeInstance*> instance_map;
	
	// Create NodeInstances
	for (auto& m : group.getNodes()){
		NodeInstance* node= &add(*m.get());
		instance_map[m.get()]= node;
		//print(debug::Ch::Nodes, debug::Vb::Moderate, "node instance created: %s", m.get()->getType().getName().cStr());
	}
	
	// Attach slots
	for (auto& comp_node : group.getNodes()){
		PROFILE();
		util::DynArray<CompositionNodeSlot*> comp_slots= comp_node->getSlots();
		for (auto& comp_slot : comp_slots){
			if (!comp_slot->isInput()) continue;

			BaseInputSlot* input_slot= instance_map[comp_node.get()]->getInputSlot(comp_slot->getIdentifier());
			if (!input_slot)
				throw resources::ResourceException(util::Str8("InputSlot not found: " + comp_slot->getIdentifier().getString()).cStr());
			
			input_slot->setExtValueReceived(&groupVars.signalsSent);
			
			for (auto& attached_info : comp_slot->getAttachedSlotInfos()){
				const CompositionNodeLogic& attached_owner= attached_info.slot->getOwner();

				ensure_msg(instance_map.find(&attached_owner) != instance_map.end(), "Instance not found: %s", attached_owner.getType().getName().cStr());
				BaseOutputSlot* output_slot= instance_map[&attached_owner]->getOutputSlot(attached_info.slot->getIdentifier());
				if (!output_slot){
					print(debug::Ch::Dev, debug::Vb::Trivial, "Has slots: %lu", (unsigned long)instance_map[&attached_owner]->getOutputSlotCount());
					throw resources::ResourceException(util::Str8("OutputSlot not found: " + attached_info.slot->getIdentifier().getString()).cStr());
				}
				output_slot->attach(attached_info.slotSub, attached_info.mySub, *input_slot);
			}
		}
	}

	if (!nodes.empty()){
		groupVars.firstOfGroup= nodes.front().get();
		groupVars.lastOfGroup= nodes.back().get();
	}

	util::OnChangeCb::trigger();
}

NodeInstance& NodeInstanceGroup::add(const CompositionNodeLogic& comp){
	PROFILE();
	NodeInstance* logic= comp.getType().createInstanceLogic(comp);
	ensure(logic);
	logic->setGroupVars(&groupVars);
	logic->create();

	nodes.pushBack(std::move(NodeInstancePtr(logic)));
	
	return *nodes.back();
}

void NodeInstanceGroup::update(){
	if (isUpdateNoop())
		return;
	
	for (auto& m : nodes){
		try {
			m->baseUpdate();
		}
		catch (const global::Exception& e){
			print(debug::Ch::Nodes, debug::Vb::Critical,
					"Node error caught: %s. Do something!",
					m->identityDump().cStr());
		}
	}
}

UpdateLine NodeInstanceGroup::getUpdateLine() const {
	PROFILE();
	util::DynArray<NodeInstance*> ptrs;
	for (const NodeInstancePtr& node : nodes){
		ptrs.pushBack(node.get());
	}
	return UpdateLine(ptrs, *this);
}

NodeInstance& NodeInstanceGroup::getNode(const util::Str8& type_name){
	for (auto& m : nodes){
		if (m->getType().getName() == type_name){
			return *m;
		}
	}
	
	throw resources::ResourceException(util::Str8::format("NodeInstanceGroup::getNode(..): %s not found", type_name.cStr()).cStr());
}

} // nodes
} // clover
