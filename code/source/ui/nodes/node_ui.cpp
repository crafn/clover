#include "node_ui.hpp"
#include "nodes/nodetype.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace ui { namespace nodes {

NodeUi::NodeUi(CompositionNodeLogic& logic_):
	logic(&logic_),
	bgPanel(util::Coord::VF(0), util::Coord::VF(0)),
	contentLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF(0)),
	titlePanel(util::Coord::VF(0), util::Coord::VF(0) + titlePanelYRad()),
	titleLabel(logic->getType().getIdentifierAsString(), util::Coord::VF(0)),
	slotLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF(0)),
	inputSlotLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF(0)),
	inputSlotGroupButton("+", util::Coord::P(0), minGroupButtonRad()),
	outputSlotLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF(0)),
	outputSlotGroupButton("+", util::Coord::P(0), minGroupButtonRad()),
	slotGroupSettingsUi(logic_){
	
	bgPanel.addSubElement(contentLayout);
	
	contentLayout.addSubElement(titlePanel);
	contentLayout.addSubElement(slotLayout);
	
	titlePanel.addSubElement(titleLabel);
	titleLabel.setTouchable(false);
	

	slotLayout.addSubElement(inputSlotLayout);
	slotLayout.addSubElement(outputSlotLayout);
	
	
	if (logic->getInputSlotTemplateGroups().empty()){
		inputSlotGroupButton.setActive(false);
		inputSlotGroupButton.setApplySuperActivation(false);
	}
	if (logic->getOutputSlotTemplateGroups().empty()){
		outputSlotGroupButton.setActive(false);
		outputSlotGroupButton.setApplySuperActivation(false);
	}
			
	inputSlotLayout.addNode(inputSlotGroupButton, 99999);
	inputSlotLayout.setIgnoreInactive();
	
	outputSlotLayout.addNode(outputSlotGroupButton, 99999);
	outputSlotLayout.setIgnoreInactive();
	
	slotGroupSettingsUi.show(false, false);
	slotGroupSettingsUi.show(false, true);
	
	createSlots();
	
	updateErrorState();
	
	listenForEvent(global::Event::OnNodeUiSlotRadiusChange);
	listenForEvent(global::Event::OnNodeSlotAdd);
	listenForEvent(global::Event::OnNodeSlotRemove);
	
	bind();
}

NodeUi::NodeUi(NodeUi&& other):
	global::EventReceiver(std::move(other)),
	logic(std::move(other.logic)),
	bgPanel(std::move(other.bgPanel)),
	contentLayout(std::move(other.contentLayout)),
	titlePanel(std::move(other.titlePanel)),
	titleLabel(std::move(other.titleLabel)),
	slotLayout(std::move(other.slotLayout)),
	inputSlotLayout(std::move(other.inputSlotLayout)),
	inputSlotGroupButton(std::move(other.inputSlotGroupButton)),
	outputSlotLayout(std::move(other.outputSlotLayout)),
	outputSlotGroupButton(std::move(other.outputSlotGroupButton)),
	slotGroupSettingsUi(std::move(other.slotGroupSettingsUi)),
	slots(std::move(other.slots)){
	
	bind();
	
}

void NodeUi::create(){
	global::Event e(global::Event::NodeUiAddLinesRequest);
	e(global::Event::Object)= this;
	e.queue();
}

void NodeUi::createSlots(){
	for (auto& m : logic->getSlots()){
		createSlot(m->getIdentifier());
	}
}

void NodeUi::bind(){
	titlePanel.setOnDraggingCallback(std::bind(&NodeUi::onMove, this, std::placeholders::_1));
	
	for (auto& m : slots){
		m->setOnSubSlotsShowCallback([&] (){
			closeOtherSubSlots(*m);
		});
	}
	
	nodeTypeChangeListener.listen(logic->getType(), [=] () {
		updateErrorState();
	});
	
	inputSlotGroupButton.setOnTriggerCallback([&] (gui::Element& e){
		slotGroupSettingsUi.setPosition(e.getPosition());
		slotGroupSettingsUi.show(true, true);
	});
	outputSlotGroupButton.setOnTriggerCallback([&] (gui::Element& e){
		slotGroupSettingsUi.setPosition(e.getPosition());
		slotGroupSettingsUi.show(true, false);
	});
	
	slotGroupSettingsUi.setOnPreSlotModifyCallback([&] (){
		//slots.clear();
	});
	
	slotGroupSettingsUi.setOnPostSlotModifyCallback([&] (){
		// Lazy: just recreate every slot..
		//createSlots();
		
		global::Event e(global::Event::NodeUiAddLinesRequest);
		e(global::Event::Object)= this;
		e.send();
	});
	
}

NodeSlotUi& NodeUi::getSlotUi(const CompositionNodeSlot& slot){
	for (auto& m : slots){
		if (m->getCompositionNodeSlot().getIdentifier() == slot.getIdentifier())
			return *m;
	}
	throw global::Exception("Slot not found: %s", slot.getName().cStr());
}

void NodeUi::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnNodeUiSlotRadiusChange:
			adjustRadius();
			getSuperGuiElement().recursiveSpatialUpdate();
		break;
		
		case global::Event::OnNodeSlotAdd: {
			if (e(global::Event::Node).getPtr<CompositionNodeLogic>() != logic)
				break;
			
			SlotIdentifier slot_id= *e(global::Event::SlotIdentifier).getPtr<SlotIdentifier>();
			createSlot(slot_id);
			
			adjustRadius();
		}
		break;
		
		case global::Event::OnNodeSlotRemove: {
			if (e(global::Event::Node).getPtr<CompositionNodeLogic>() != logic)
				break;
			
			SlotIdentifier slot_id= *e(global::Event::SlotIdentifier).getPtr<SlotIdentifier>();
			destroySlot(slot_id);
			
			adjustRadius();
		}
		break;
		
		default:;
		
	}
}

void NodeUi::closeFloatings(){
	for (auto& m : slots){
		m->showSubSlots(false);
	}
}

void NodeUi::update(){
	logic->setPosition(getSuperGuiElement().getOffset().converted(util::Coord::View_Pixels).getValue());
	
	for (auto& m : slots){
		m->update();
	}
}

void NodeUi::adjustRadius(){
	
	// First reset
	inputSlotLayout.setMinRadius(util::Coord::P(0));
	outputSlotLayout.setMinRadius(util::Coord::P(0));
	
	for (auto& m : slots){
		m->resetHorizontalColumnRadius();
	}
	
	// Then calculate
	inputSlotLayout.minimizeRadius();
	outputSlotLayout.minimizeRadius();
	
	util::Coord input_min= inputSlotLayout.getRadius();
	util::Coord output_min= outputSlotLayout.getRadius();

	const real64 additional_gap= 0.01;
	input_min.x += additional_gap;
	output_min.x += additional_gap;
	
	for (auto& m : slots){
		if (m->getCompositionNodeSlot().getIdentifier().input){
			m->setHorizontalColumnRadius(input_min);
		}
		else {
			m->setHorizontalColumnRadius(output_min);
		}
	}
	
	
	// Group buttons
	util::Coord input_group_button_rad= input_min.onlyX() + minGroupButtonRad().onlyY();
	input_group_button_rad.x -= additional_gap*2;
	real64 btn_min_x= minGroupButtonRad().converted(input_group_button_rad.getType()).getValue().x;
	if (input_group_button_rad.x < btn_min_x) input_group_button_rad.x= btn_min_x;
	inputSlotGroupButton.setMaxRadius(input_group_button_rad);
	inputSlotGroupButton.setRadius(input_group_button_rad);
	
	util::Coord output_group_button_rad= output_min.onlyX() + minGroupButtonRad().onlyY();
	output_group_button_rad.x -= additional_gap*2;
	if (output_group_button_rad.x < btn_min_x) output_group_button_rad.x= btn_min_x;
	outputSlotGroupButton.setMaxRadius(output_group_button_rad);
	outputSlotGroupButton.setRadius(output_group_button_rad);
	
	
	inputSlotLayout.setMinRadius(input_min);
	inputSlotLayout.setMaxRadius(input_min);
	inputSlotLayout.setRadius(input_min);
	
	outputSlotLayout.setMinRadius(output_min);
	outputSlotLayout.setMaxRadius(output_min);
	outputSlotLayout.setRadius(output_min);
	
	util::Coord min= input_min + output_min.onlyX();
	if (min.y < output_min.y) min.y= output_min.y;
	
	slotLayout.setRadius(min);
	slotLayout.setMaxRadius(min);
	
	util::Coord content_rad= slotLayout.getRadius() + titlePanelYRad();
	contentLayout.setRadius(content_rad);
	contentLayout.setMaxRadius(content_rad);
	
	util::Coord rad_x= content_rad.onlyX();
	
	titlePanel.setMaxRadius(titlePanelYRad() + rad_x);
	titlePanel.setRadius(titlePanelYRad() + rad_x);
	
	bgPanel.setRadius(content_rad);
	bgPanel.setMaxRadius(content_rad);
	
}

void NodeUi::closeOtherSubSlots(const NodeSlotUi& ui){
	for (auto& m : slots){
		if (m.get() != &ui){
			m->showSubSlots(false);
		}
	}
}

void NodeUi::onMove(gui::Element& e){

	util::Coord offset= bgPanel.getOffset();
	offset += gUserInput->getCursorDifference().converted(util::Coord::View_Stretch);
	bgPanel.setOffset(offset);
	bgPanel.bringTop();
	
	
	bgPanel.recursiveSpatialUpdate();

	global::Event evt(global::Event::OnNodeUiMove);
	evt(global::Event::Object)= this;
	evt.send();
	
}

NodeSlotUi& NodeUi::createSlot(const SlotIdentifier& slot_id){
	
	for (auto& m : slots){
		ensure_msg(m->getCompositionNodeSlot()->getIdentifier() != slot_id, "Duplicate slot: %s", slot_id.getString().cStr());
	}
	
	slots.pushBack(std::move(NodeSlotUiPtr(
		new NodeSlotUi(*this, CompositionNodeSlotHandle(logic->getSlot(slot_id)))
		)));

	if (slot_id.input){
		inputSlotLayout.addSubElement(slots.back()->getSuperGuiElement());
	}
	else {
		outputSlotLayout.addSubElement(slots.back()->getSuperGuiElement());
	}
	
	adjustRadius();
	
	return *slots.back();
}

void NodeUi::destroySlot(const SlotIdentifier& slot_id){
	
	for (auto it= slots.begin(); it != slots.end(); ++it){
		if (it->get()->getCompositionNodeSlot().getIdentifier() == slot_id){
			slots.erase(it);
			return;
		}
	}
	
	throw resources::ResourceException("Slot not found: %s", slot_id.getString().cStr());
}

void NodeUi::updateErrorState(){
	bgPanel.setErrorVisuals(logic->getType().getResourceState() == resources::Resource::State::Error);
}

}} // ui::nodes
} // clover
