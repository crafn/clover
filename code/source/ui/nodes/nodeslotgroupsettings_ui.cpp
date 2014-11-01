#include "nodeslotgroupsettings_ui.hpp"
#include "nodeslot_ui.hpp"

namespace clover {
namespace ui { namespace nodes {

NodeSlotGroupSettingsUi::NodeSlotGroupSettingsUi(CompositionNodeLogic& comp_node):
		compositionNodeLogic(&comp_node),
		layout(gui::LinearLayoutElement::Vertical),
		isInput(false){
	
	floating.addSubElement(bgPanel);
	bgPanel.addSubElement(layout);
	
	floating.setAutoResize();
	bgPanel.setAutoResize();
	layout.setAutoResize();
	
	bindCallbacks();
}

NodeSlotGroupSettingsUi::NodeSlotGroupSettingsUi(NodeSlotGroupSettingsUi&& other):
	compositionNodeLogic(std::move(other.compositionNodeLogic)),
	floating(std::move(other.floating)),
	bgPanel(std::move(other.bgPanel)),
	layout(std::move(other.layout)),
	onPreSlotModify(std::move(other.onPreSlotModify)),
	onPostSlotModify(std::move(other.onPostSlotModify)),
	slotCheckBoxes(std::move(other.slotCheckBoxes)){
	
	bindCallbacks();
}

void NodeSlotGroupSettingsUi::show(bool true_to_show, bool is_input){
	floating.setActive(true_to_show);
	
	slotCheckBoxes.clear();
	variantAddButtons.clear();
		
	if (true_to_show){
		isInput= is_input;
		
		for (auto& group : compositionNodeLogic->getSlotTemplateGroups(is_input)){
			ensure(group);
			addGroupGuiElements(*group);
		}
		bindCallbacks();
	}
}

void NodeSlotGroupSettingsUi::setPosition(const util::Coord& pos){
	floating.setPosition(pos);
}

void NodeSlotGroupSettingsUi::addCheckBox(bool checked, const SlotTemplate& slot){
	const util::Str8& group_name= slot.getOwner().getName();
	
	gui::CheckBoxElement chkbox(group_name + " - " + slot.getName());
	gui::TextLabelElement label= std::move(NodeSlotGui::addSignalTypeLabel(slot.getSignalType(), chkbox));
	
	chkbox.setChecked(checked);
	
	slotCheckBoxes.pushBack(std::move(CheckBoxWrap{
		std::move(chkbox),
		std::move(label),
		slot}));
	layout.addNode(slotCheckBoxes.back().element);
}

void NodeSlotGroupSettingsUi::addVariantAddButton(CompositionNodeSlotTemplateGroup& group){
	ensure(group.isVariant());
	
	gui::ButtonElement btn("+ " + group.getName() + " slot");

	variantAddButtons.pushBack(std::move(VariantAddButtonWrap{
		std::move(btn),
		&group}));
	
	layout.addNode(variantAddButtons.back().element);
}

void NodeSlotGroupSettingsUi::addGroupGuiElements(CompositionNodeSlotTemplateGroup& group){
	if (group.isVariant()){
		// Add every variant slot in compositionNode as a checkbox so that they can be removed from node
		for (auto& slot : compositionNodeLogic->getSlots()){
			SlotIdentifier slot_id= slot->getIdentifier();
			if (!group.hasSlotTemplate(slot_id) && // Predefined slot templates are added elsewhere
				slot_id.groupName == group.getName() &&
				slot_id.input == group.isInput()){
				
				addCheckBox(true, SlotTemplate(group, slot_id.name, slot_id.signalType, slot_id.input));
			}
		}
		
		// Button for adding new variant nodes
		addVariantAddButton(group);
	}
	
	// Add checkbox for every slot template (even if group is variant)
	for (auto& slot : group.getSlots()){
		addCheckBox(compositionNodeLogic->hasSlot(slot.getIdentifier()), slot);
	}
	
}

void NodeSlotGroupSettingsUi::showCreateSlotDialog(CompositionNodeSlotTemplateGroup& group){
	ensure(compositionNodeLogic);
	
	createSlotDialog= util::makeUniquePtr<CreateNodeSlotDialog>();
	
	createSlotDialog->setOnCloseCallback([&] (SignalType type, util::Str8 name){
		if (type != SignalType::None){
			SlotTemplate slot= SlotTemplate{group, name, type, isInput};
			compositionNodeLogic->addSlot(slot);
			
			if (group.isMirroring())
				compositionNodeLogic->addSlot(slot.mirrored());
		}
		createSlotDialog.reset();
	});
}

void NodeSlotGroupSettingsUi::bindCallbacks(){
	floating.setOnOutTriggerCallback([&] (gui::FloatingElement& e){
		e.setActive(false);
	});

	for (auto& m : slotCheckBoxes){
		m.element.setOnValueModifyCallback([&] (gui::CheckBoxElement& e){
			SlotTemplate& slot= m.slotTemplate;
			
			if (onPreSlotModify)
				onPreSlotModify();
			
			if (e.isChecked()){
				// Add slot
				print(debug::Ch::Dev, debug::Vb::Trivial, "Adding template slot: %s", slot.getIdentifier().getString().cStr());
				compositionNodeLogic->addSlot(slot);
				
				if (slot.getOwner().isMirroring())
					compositionNodeLogic->addSlot(slot.mirrored());
			}
			else {
				// Remove slot
				print(debug::Ch::Dev, debug::Vb::Trivial, "Removing template slot: %s", slot.getIdentifier().getString().cStr());
				compositionNodeLogic->removeSlot(slot);
				
				if (slot.getOwner().isMirroring())
					compositionNodeLogic->removeSlot(slot.mirrored());
			}
			
			if (onPostSlotModify)
				onPostSlotModify();
		});
	}
	
	for (auto& m : variantAddButtons){
		m.element.setOnTriggerCallback([&] (gui::Element& e){
			ensure(m.group);
			showCreateSlotDialog(*m.group);
		});
	}
}

}} // ui::nodes
} // clover