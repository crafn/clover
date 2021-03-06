#include "nodeinstance_actionlistener.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* ActionListenerNodeInstance::compNode()
{
	CompositionNodeLogic* n= new CompositionNodeLogic{};
	n->addInputSlot("channelName", SignalType::String, util::Str8("player0")); // Channel to listen
	n->addInputSlot("tag", SignalType::String, util::Str8("")); // Tag to listen
	n->addInputSlot("actionName", SignalType::String, util::Str8(""));
	n->addOutputSlot("onAction", SignalType::Trigger); // Triggered on action

	CompositionNodeSlotTemplateGroup& attribs_group= n->addOutputSlotTemplateGroup("value");
	attribs_group.setAsVariant(true);
	return n;
}

void ActionListenerNodeInstance::create(){
	channelNameIn= addInputSlot<SignalType::String>("channelName");
	tagIn= addInputSlot<SignalType::String>("tag");
	actionNameIn= addInputSlot<SignalType::String>("actionName");

	onActionOut= addOutputSlot<SignalType::Trigger>("onAction");

	for (const auto& comp_slot : compositionNodeLogic->getTemplateGroupSlots()){
		BaseOutputSlot* slot= addOutputSlot(	comp_slot->getName(),
												comp_slot->getTemplateGroupName(),
												comp_slot->getSignalType());
		valueOutputs[comp_slot->getSignalType()]= slot;
	}

	auto update_action_listener= +[] (ActionListenerNodeInstance* self) {
		self->actionListener=
			ui::hid::ActionListener<>(	self->channelNameIn->get(),
										self->tagIn->get(),
										self->actionNameIn->get(),
										[self] (nodes::SignalValue v){
			self->onActionOut->send();

			if (!self->valueOutputs.empty()){
				auto it= self->valueOutputs.find(v.getType());
				if (it != self->valueOutputs.end()){
					it->second->send(v.getAny());
				}
				else {
					print(debug::Ch::Nodes, debug::Vb::Moderate, 
						"Action value type didn't match with slot: %s", self->actionNameIn->get().cStr());
				}
			}
		});
	};

	channelNameIn->setOnReceiveCallback(update_action_listener);
	tagIn->setOnReceiveCallback(update_action_listener);
	actionNameIn->setOnReceiveCallback(update_action_listener);

	setUpdateNeeded(false);
}

} // nodes
} // clover
