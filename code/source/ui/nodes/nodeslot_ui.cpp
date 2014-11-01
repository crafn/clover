#include "nodeslot_ui.hpp"
#include "debug/debugdraw.hpp"
#include "gui/gui_mgr.hpp"
#include "signalvalue_ui_factory.hpp"

namespace clover {
namespace ui { namespace nodes {

gui::TextLabelElement NodeSlotGui::addSignalTypeLabel(SignalType s, gui::Element& e){
	util::Str8 str;
	util::Coord pos= e.getRadius().onlyY()*0.85;
	util::Coord rad= util::Coord::P(4);
	
	str= RuntimeSignalTypeTraits::enumString(s);
	
	gui::TextLabelElement label(str, pos, rad);
	label.setType(gui::TextLabelElement::Type::Dev_Light);
	
	e.addSubElement(label);
	
	return (label);
}

NodeSlotGui::NodeSlotGui(NodeSlotUi& o, SubSignalType sub_type):
	owner(&o),
	subType(sub_type),
	bgLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF(0)),
	bgPanel(util::Coord::VF(0), util::Coord::VF(0)),
	contentLayout(gui::LinearLayoutElement::Horizontal, util::Coord::VF(0), util::Coord::VF(0)),
	label("Name", util::Coord::VF(0)),
	signalTypeLabel("Value", util::Coord::VF(0)){
	
	addSubElement(bgLayout);
	bgLayout.setMargin(false);
	bool is_output= !owner->getCompositionNodeSlot()->isInput();
	if (is_output){
		bgLayout.mirrorFirstNodeSide();
	}

	
	bgLayout.addSubElement(bgPanel);
	bgPanel.setTouchable(false);
	
	bgPanel.addSubElement(contentLayout);
	
	contentLayout.addSubElement(label);
	label.setTouchable(false);
	
	updateLabel();
	
	if (subType == SubSignalType::None){
		// Normal signal slot

		signalTypeLabel= std::move(addSignalTypeLabel(o.getCompositionNodeSlot()->getSignalType(), bgPanel));
	}
	else {
		// Subtype slot
		label.setText(RuntimeSubSignalTypeTraits::name(subType));
		signalTypeLabel= std::move(addSignalTypeLabel(RuntimeSubSignalTypeTraits::signalType(subType), bgPanel));
	}
	
	if (!is_output && subType == SubSignalType::None){
		signalValueUi= std::unique_ptr<BaseSignalValueUi>(
			SignalValueUiFactory::create(owner->getCompositionNodeSlot()->getSignalType()));
		
		contentLayout.addSubElement(signalValueUi->getViewGuiElement());
		
		signalValueUi->setValue(owner->getCompositionNodeSlot()->getDefaultValue(),
								!owner->getCompositionNodeSlot()->isDefaultValueSet());
	}
	
	
	
	adjustRadius();
	
}

NodeSlotGui::~NodeSlotGui(){
	if (getOwner().getCompositionNodeSlot().getIdentifier().input)
		detachLines();
	
	while(!arrivingLineSlots.empty()){
		arrivingLineSlots.back()->detachLine(*this);
	}
	
	destroyVisuals();
}

void NodeSlotGui::adjustRadius(){

	util::Coord rad=	util::Coord::VF({0.05, 0.005}) + 
				label.getRadius().onlyX() + 
				label.getRadius().onlyY();
	
	if (signalValueUi){
		rad= rad + signalValueUi->getViewGuiElement().getRadius().onlyX();
	}
	
	bgLayout.setMinRadius(rad);
	bgLayout.setMaxRadius(rad);
	bgLayout.setRadius(rad);
	
	contentLayout.setMinRadius(rad);
	contentLayout.setMaxRadius(rad);
	contentLayout.setRadius(rad);
	
	setMinRadius(rad);
	setMaxRadius(rad);
	setRadius(rad);
	
	util::Coord panel_rad= rad;
	panel_rad.y *= 0.9;
	
	bgPanel.setMinRadius(panel_rad);
	bgPanel.setRadius(panel_rad);
	bgPanel.setMaxRadius(panel_rad);
	
	//updateLines();
}

void NodeSlotGui::attachLine(NodeSlotGui& other, bool proxy){
	ensure(owner->getCompositionNodeSlot()->isInput());
	
	lines.emplaceBack(other, *this, *createVisualEntity(gui::ElementVisualEntity::Type::Custom), proxy);
	
	lines.back().create();
	lines.back().generate();
	
	other.addArrivingLineSlot(*this);
	
	if (!isActive()){
		lines.back().getElementVisualEntity().setActiveTarget(false);
	}
}

void NodeSlotGui::detachLine(NodeSlotGui& other){
	ensure(getOwner().getCompositionNodeSlot()->isInput());
	
	//print(debug::Ch::General, debug::Vb::Trivial, "detachLine this %s %p other %s %p",
	//	getOwner().getCompositionNodeSlot().getIdentifier().getString().cStr(), this,
	//	other.getOwner().getCompositionNodeSlot().getIdentifier().getString().cStr(), &other);
	
	for (auto it= lines.begin(); it != lines.end(); ++it){
		//print(debug::Ch::General, debug::Vb::Trivial, "    start %p", &it->getStartSlot());
		ensure(&it->getStartSlot());
		if (&it->getStartSlot() == &other){
			it->getStartSlot().removeArrivingLineSlot(*this);
			destroyVisualEntity(&it->getElementVisualEntity());
			lines.erase(it);
			return;
		}
	}
	
	throw global::Exception("Line not found");
}

void NodeSlotGui::detachLines(bool only_proxy){
	ensure(owner->getCompositionNodeSlot().getIdentifier().input);
	
	for (auto it= lines.begin(); it != lines.end();){
		
		if (!only_proxy || it->isProxy()){
			it->getStartSlot().removeArrivingLineSlot(*this);
			destroyVisualEntity(&it->getElementVisualEntity());
			it= lines.erase(it);
		}
		else {
			++it;
		}
	}

}



NodeSlotGui* NodeSlotGui::getAttached(){
	for (auto& m : lines){
		if (!m.isProxy()) return &m.getStartSlot();
	}
	
	return nullptr;
}


void NodeSlotGui::updateLines(){
	for (auto& m : lines){
		m.generate();
	}
}

void NodeSlotGui::setRadius(const util::Coord& r){
	
	setMinRadius(r);
	setMaxRadius(r);
	BaseType::setRadius(r);
	
	bgLayout.setMinRadius(r);
	bgLayout.setMaxRadius(r);
	bgLayout.setRadius(r);
	
}


void NodeSlotGui::spatialUpdate(){
	BaseType::spatialUpdate();
	
	for (auto& m : lines){
		m.spatialUpdate();
	}
	
	// Don't update lines when dragging nodes, because some nodes are spatialUpdated
	// before/after lines and that makes a bad looking delay
	if (!guiCursor->getDraggedElement())
		updateLines();
}

void NodeSlotGui::setActive(bool b){
	// NodeSlotUi needs this, because guiSlot contains subSlots
	BaseType::setActive(b);
	
	if (!b){
		for (auto& m : subElements){
			m->setActive(false);
		}
	}
}

void NodeSlotGui::updateLabel(){
	util::Str8 prefix= "";
	util::Str8 postfix= "";
	
	if (owner->getCompositionNodeSlot().getIdentifier().groupName != ""){
		prefix= owner->getCompositionNodeSlot().getIdentifier().groupName + " - ";
	}
	
	if (owner->getCompositionNodeSlot()->isInput() && 
		owner->getCompositionNodeSlot()->isDefaultValueSet()){
		
		postfix= " |";
	}
	
	label.setText(prefix + owner->getCompositionNodeSlot()->getName() + postfix);
	
	adjustRadius();
}

NodeSlotGui::Line::Line(NodeSlotGui& start, NodeSlotGui& end, gui::ElementVisualEntity& entity, bool proxy_):
	startSlot(&start),
	endSlot(&end),
	startIsMoved(false),
	elementVisualEntity(&entity),
	proxy(proxy_){
	ensure(!start.getOwner().getCompositionNodeSlot()->isInput());
	ensure(end.getOwner().getCompositionNodeSlot()->isInput());
}

NodeSlotGui::Line::~Line(){
	elementVisualEntity->clear();
}

void NodeSlotGui::Line::create(){
	model.setMesh(mesh);
	modelEntityDef.setModel(model);
	elementVisualEntity->getVisualEntity().setDef(modelEntityDef);
}

void NodeSlotGui::Line::generate(){
	NodeSlotGui* start_slot= startSlot;
	
	if (startIsMoved){
		// Use the actual slot instead of a subslot if subslot is hidden
		start_slot= &startSlot->getOwner().getSuperGuiElement();
	}
	
	util::Coord start= start_slot->getPosition() - endSlot->getPosition() + start_slot->getRadius().onlyX();
	util::Coord end= util::Coord(0, start_slot->getPosition().getType()) - endSlot->getRadius().onlyX();
	
	util::Vec2f start_v= start.converted(util::Coord::View_Pixels).getValue().casted<util::Vec2f>();
	util::Vec2f end_v= end.converted(util::Coord::View_Pixels).getValue().casted<util::Vec2f>();
	
	// Don't generate if nothing has changed
	if (start_v.distanceSqr(lastStart) <= 0.000001f && end_v.distanceSqr(lastEnd) <= 0.000001f)
		return;
		
	lastStart= start_v;
	lastEnd= end_v;
	
	generateMesh(start_v, end_v);
	
	if (proxy || startIsMoved)
		modelEntityDef.setEnvLight(0.5);
	else
		modelEntityDef.setEnvLight(1.0);
	
	elementVisualEntity->getVisualEntity().setScale(util::Vec3d(1.0));
	elementVisualEntity->getVisualEntity().setScaleCoordinateSpace(util::Coord::View_Pixels);
}


void NodeSlotGui::Line::spatialUpdate(){
	
	if (startSlot->isActive() == startIsMoved){
		// Change the start position if subSlotFloating has been closed
		startIsMoved= !startSlot->isActive();
	}
	
	elementVisualEntity->setPositionTarget(endSlot->getPosition().converted(util::Coord::View_Fit));
}

void NodeSlotGui::Line::generateMesh(util::Vec2f start, util::Vec2f end){
	util::Vec2d start_d= start.converted<util::Vec2d>();
	util::Vec2d end_d= end.converted<util::Vec2d>();
	
	util::Vec2d control_offset;
	control_offset.x= util::limited((start_d.x - end_d.x), 0.0, 1500.0)*3.0 + 150.0;
	
	const real64 fully_straight_dist= 10.0;
	const real64 start_straighten_dist= 40.0;
	real64 dist= start_d.distance(end_d);
	if (dist < start_straighten_dist){
		// Straighten when start and end are close together
		control_offset.x *= util::limited(
			(dist - fully_straight_dist)/(start_straighten_dist - fully_straight_dist),
			0.0,
			1.0);
	}
	
	auto point_on_curve= [start_d, end_d, control_offset](real64 t) -> util::Vec2d {
		return util::geom::pointOnCatmull(t, start_d - control_offset, start_d, end_d, end_d + control_offset);
	};
	
	// Calculate points on curve
	util::DynArray<util::Vec2d> points;
	const SizeType point_count= 30;
	for (SizeType i= 0; i < point_count; ++i){
		real64 t= (real64)i/(point_count - 1);
		points.pushBack(point_on_curve(t));
	}
	
	// Create mesh from curve points
	mesh.clear();
	mesh.add(util::Polygon::extrudedLineSegment(points, 0.7).triangulated().converted<visual::TriMesh>());
	mesh.setColor(util::Color{1,1,1,0.4});
	
	// Add triangle indicating direction
	visual::TriMesh tri_mesh;
	util::Vec2f tri_tip= (point_on_curve(0.55) - point_on_curve(0.45)).converted<util::Vec2f>().normalized()*10.0;
	util::Vec2f tri_center= point_on_curve(0.5).converted<util::Vec2f>();
	tri_mesh.addTriangle(	tri_center + tri_tip,
							tri_center + tri_tip.rotated(util::tau/3.0)*0.7,
							tri_center + tri_tip.rotated(-util::tau/3.0)*0.7);
	tri_mesh.setColor(util::Color{1.0, 1.0, 1.0, 0.2});
	mesh.add(tri_mesh);
	
	mesh.flush();
}

void NodeSlotGui::onEvent(global::Event& e){
	switch(e.getType()){

		default:;
	}
}

void NodeSlotGui::addArrivingLineSlot(NodeSlotGui& slot){
	ensure(!owner->getCompositionNodeSlot().getIdentifier().input);
	arrivingLineSlots.pushBack(&slot);
	
	//print(debug::Ch::General, debug::Vb::Trivial, "Slot: %p added to %p", &slot, this);
}

void NodeSlotGui::removeArrivingLineSlot(NodeSlotGui& slot){
	ensure(!owner->getCompositionNodeSlot().getIdentifier().input);
	//print(debug::Ch::General, debug::Vb::Trivial, "Slot: %p removing from %p", &slot, this);
	
	auto it= arrivingLineSlots.find(&slot);
	ensure(it != arrivingLineSlots.end())
	arrivingLineSlots.erase(it);
}

NodeSlotUi::NodeSlotUi(const NodeUi& owner_, const CompositionNodeSlotHandle& comp_slot):
	owner(&owner_),
	compositionNodeSlot(comp_slot),
	slotGui(*this),
	subSlotFloating(util::Coord::VF(0)),
	subSlotLayout(gui::LinearLayoutElement::Vertical, util::Coord::VF(0), util::Coord::VF(0)){
		
	subSlotFloating.setActive(false);
	subSlotFloating.setApplySuperActivation(false);
	subSlotFloating.addSubElement(subSlotLayout);
	
	slotGui.addSubElement(subSlotFloating);
	
	auto subsignal_types= RuntimeSignalTypeTraits::subTypes(compositionNodeSlot->getSignalType());
	
	util::Coord subslot_min= util::Coord::VF(0);
	for (auto& m : subsignal_types){
		
		subSlots.pushBack(std::move(NodeSlotGuiPtr(new NodeSlotGui(*this, m))));
		subSlotLayout.addSubElement(*subSlots.back());
		
		util::Coord::Type t= subSlots.back()->getRadius().getType();
		if (subSlots.back()->getRadius().x > subslot_min.converted(t).x)
			subslot_min= subSlots.back()->getRadius();
	}
	
	for (auto& m : subSlots){
		m->setRadius(m->getRadius().onlyY() + subslot_min.onlyX());
	}
	
	bind();
	listenForEvent(global::Event::OnNodeUiMove);
}
/*
NodeSlotUi::NodeSlotUi(NodeSlotUi&& other):
	global::EventReceiver(std::move(other)),
	owner(other.owner),
	compositionNodeSlot(std::move(other.compositionNodeSlot)),
	slotGui(std::move(other.slotGui)),
	subSlotFloating(std::move(other.subSlotFloating)),
	subSlotLayout(std::move(other.subSlotLayout)),
	subSlots(std::move(other.subSlots)),
	onSubSlotsShow(std::move(other.onSubSlotsShow)){
	
	slotGui.setOwner(*this);
	
	for (auto& m : subSlots){
		m.setOwner(*this);
	}
		
	bind();
	
}*/

NodeSlotUi::~NodeSlotUi(){

}

void NodeSlotUi::bind(){
	
	slotGui.setOnDraggingStopCallback(std::bind(&NodeSlotUi::onDraggingStop, this, std::placeholders::_1));
	slotGui.setOnDraggingCallback(std::bind(&NodeSlotUi::onDragging, this, std::placeholders::_1));
	
	slotGui.setOnTriggerCallback([&] (gui::Element&){
		// Show subSlots when clicked
		showSubSlots(!isSubSlotsShowing());
		if (onSubSlotsShow)
				onSubSlotsShow();
	});
	
	for (auto& m : subSlots){
		m->setOnDraggingStopCallback(std::bind(&NodeSlotUi::onDraggingStop, this, std::placeholders::_1));
		m->setOnDraggingCallback(std::bind(&NodeSlotUi::onDragging, this, std::placeholders::_1));
	}
	
	slotGui.setOnSecondaryTriggerCallback([&] (gui::Element&){
		if (slotGui.getSignalValueUi())
			slotGui.getSignalValueUi()->showEditControls(!slotGui.getSignalValueUi()->isEditControlsShowing());
	});
	
	if (slotGui.getSignalValueUi()){
		slotGui.getSignalValueUi()->setOnValueModifyCallback([&] () {
			// util::Set default value of the slot from gui
			compositionNodeSlot->setDefaultValue(slotGui.getSignalValueUi()->getValue());
			slotGui.updateLabel();
			
			global::Event e(global::Event::OnNodeUiSlotRadiusChange);
			e(global::Event::Object)= this;
			e.send();
		});
		
		slotGui.getSignalValueUi()->setOnValueUnsetCallback([&] (){
			compositionNodeSlot->unsetDefaultValue();
			slotGui.updateLabel();
			
			global::Event e(global::Event::OnNodeUiSlotRadiusChange);
			e(global::Event::Object)= this;
			e.send();
		});
	}
}


void NodeSlotUi::setHorizontalColumnRadius(const util::Coord& r){		
	slotGui.setRadius(slotGui.getRadius().onlyY() + r.onlyX());
}

void NodeSlotUi::resetHorizontalColumnRadius(){
	slotGui.adjustRadius();
}

void NodeSlotUi::showSubSlots(bool b){
	if (subSlotFloating.isActive() == b) return;
	
	if (compositionNodeSlot->isInput()){
		if (b){
			slotGui.detachLines(true);
		}
		else {
			// Show proxy lines (= subslot lines)
			for (auto& m : subSlots){
				if (m->getAttached())
					slotGui.attachLine(*m->getAttached(), true);
			}
		}
	}
	
	subSlotLayout.minimizeRadius();
	
	real32 side_mul= -1;
	if (!compositionNodeSlot->isInput())
		side_mul= 1;
		
	subSlotFloating.setOffset( (subSlotFloating.getRadius().onlyX() + slotGui.getRadius().onlyX())*side_mul);
	subSlotFloating.setActive(b);
	subSlotFloating.recursiveSpatialUpdate();

	for (auto& m : subSlots){
		m->updateLines();
	}
	
}


void NodeSlotUi::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnNodeUiMove:
			updateLines();
		break;
		 
		default:;
	}
}

void NodeSlotUi::attachLine(NodeSlotUi& output, SubSignalType from_sub, SubSignalType to_sub){
	ensure(getCompositionNodeSlot()->isInput());
	NodeSlotGui& from_gui= output.getSlotGui(from_sub);
	NodeSlotGui& to_gui= getSlotGui(to_sub);
	
	to_gui.attachLine(from_gui);

	if (!to_gui.isShown()){
		// If is connected to a subslot and the gui is hidden, create a proxy which is shown
		to_gui.getOwner().getSlotGui(SubSignalType::None).attachLine(from_gui, true);
	}


}

void NodeSlotUi::updateLines(){	
	slotGui.updateLines();
	
	for (auto& m : subSlots){
		m->updateLines();
	}
}

NodeSlotGui& NodeSlotUi::getSlotGui(SubSignalType s){
	if (s == SubSignalType::None) return slotGui;
	
	for (auto& m : subSlots){
		if (m->getSubSignalType() == s)
			return *m;
	}
	
	throw global::Exception("SubSignalType %s not found", s);
}

void NodeSlotUi::onDragging(gui::Element& e){
	NodeSlotGui* under= dynamic_cast<NodeSlotGui*>(gui::gGuiMgr->getCursor().getUnderElement());
	NodeSlotGui* from= dynamic_cast<NodeSlotGui*>(&e);
	
	ensure(from);
	
	if (!under || under == &e || under->getSubSignalType() != SubSignalType::None) return;
	
	bool from_is_subslot= from->getSubSignalType() != SubSignalType::None;
	
	SignalType type_from= getCompositionNodeSlot()->getSignalType();
	if (from_is_subslot){
		type_from= RuntimeSubSignalTypeTraits::signalType(from->getSubSignalType());
	}
	SignalType type_under= under->getOwner().getCompositionNodeSlot()->getSignalType();
	
	bool is_same= under->getOwner().getCompositionNodeSlot()->getSignalType() == type_from;
	
	bool prevent= false;
	
	if (!is_same){
		// Cancel subslot showing if under doesn't contain from slot
		
		auto subslot_list= RuntimeSignalTypeTraits::subTypes(type_under);
		bool found= false;
		for (auto& m : subslot_list){
			if (RuntimeSubSignalTypeTraits::signalType(m) == type_from){
				found= true;
				break;
			}
		}
	
		if (!found){
			prevent= true;
		}
	}
	
	if (!is_same && !prevent){
		under->getOwner().showSubSlots();
		if (under->getOwner().onSubSlotsShow)
			under->getOwner().onSubSlotsShow();
	}
}

void NodeSlotUi::onDraggingStop(gui::Element& e){
	NodeSlotUi* dragged_to= nullptr;
	NodeSlotUi* dragged_from= nullptr;
	
	NodeSlotGui* dragged_from_gui= dynamic_cast<NodeSlotGui*>(&e);
	if (!dragged_from_gui) return;
		
	dragged_from= &dragged_from_gui->getOwner();
		
	NodeSlotGui* dragged_to_gui= nullptr;
	
	
	if (gui::gGuiMgr->getCursor().getTouchedElement()){
		dragged_to_gui= dynamic_cast<NodeSlotGui*>(
			gui::gGuiMgr->getCursor().getUnderElement());
		
		if (dragged_to_gui)
			dragged_to= &dragged_to_gui->getOwner();
	}
	
	ensure(dragged_from && dragged_from_gui);
	
	bool valid_dragging= false;

	if (dragged_from != dragged_to && dragged_to){
		// Dragged on a slot

		ensure(dragged_from->getCompositionNodeSlot() != dragged_to->getCompositionNodeSlot());
		
		// Connecting slots
		
		if (dragged_from->getCompositionNodeSlot()->isInput() != 
				dragged_to->getCompositionNodeSlot()->isInput() &&
			dragged_from->getCompositionNodeSlot()->isCompatible(
				dragged_from_gui->getSubSignalType(),
				dragged_to_gui->getSubSignalType(),
				*dragged_to->getCompositionNodeSlot().get())){
			

			NodeSlotUi* output= dragged_from;
			NodeSlotUi* input= dragged_to;
			
			NodeSlotGui* output_gui= dragged_from_gui;
			NodeSlotGui* input_gui= dragged_to_gui;
			
			if (output->getCompositionNodeSlot()->isInput()){
				std::swap(output, input);
				std::swap(output_gui, input_gui);
			}
			
			if (!input->getCompositionNodeSlot()->isAttached(input_gui->getSubSignalType()) &&
				!input->getCompositionNodeSlot()->isAttached(SubSignalType::None) &&
				!(input_gui->getSubSignalType() == SubSignalType::None && input->getCompositionNodeSlot()->isAttached())){
					
				print(debug::Ch::Dev, debug::Vb::Trivial, "Connected %s to %s",
					dragged_from->getCompositionNodeSlot()->getName().cStr(),
					dragged_to->getCompositionNodeSlot()->getName().cStr());
					
				output->getCompositionNodeSlot()->attach(
					*input->getCompositionNodeSlot().get(),
					output_gui->getSubSignalType(),
					input_gui->getSubSignalType());
				
				valid_dragging= true;
				
				input_gui->attachLine(*output_gui);
			
			}
			
		}
	}
	
	if (!valid_dragging && dragged_from != dragged_to && dragged_from->getCompositionNodeSlot()->isInput()){
		// Disconnect
		
		if (dragged_from_gui->getAttached()){
			NodeSlotGui* attached_gui= dragged_from_gui->getAttached();
		
			dragged_from->getCompositionNodeSlot()->detach(
				*dragged_from_gui->getAttached()->getOwner().getCompositionNodeSlot().get(),
				dragged_from_gui->getSubSignalType(),
				attached_gui->getSubSignalType());
			
			
			dragged_from_gui->detachLines();
		
		}

	}

}

}} // ui::nodes
} // clover