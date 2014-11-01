#include "ec_ui_nodes.hpp"
#include "gui/gui_mgr.hpp"
#include "hardware/keyboard.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

NodesEcUi::NodesEcUi(NodesEc& comp)
	: BaseType(comp)
	, nodeListView(util::Coord::VF(0), util::Coord::VF(0.2))
	, nodeMovedInLastFrame(false)
	, showNodeListListener("host", "dev", "showNodeList", [this] (){ if (getContentElement().isActive()) showNodeList(); })
	, deleteNodeListener("host", "dev", "deleteNode", [this] (){ if (getContentElement().isActive()) deleteNode(); }){
	
	getComponent().addOnResourceSelectCallback([&] (){
		nodeUis.clear();
		if (getSelectedResource())
			addNodeUis();
	});
		
	getContentElement().setDraggingType(gui::Element::DraggingType::Middle);
		
	getContentElement().setOnDraggingCallback([&] (gui::Element&){
		// Move nodes when dragging background
		
		for (auto& m : nodeUis)
			m.getSuperGuiElement().setOffset(m.getSuperGuiElement().getOffset() + gUserInput->getCursorDifference());
		
		updateNodeVisibility();
	});
	
	// util::Set up node list
	
	util::DynArray<util::Str8> names= getComponent().getNodeTypes();
	std::sort(names.begin(), names.end());
	for (auto& m : names){
		nodeListView.append(m);
	}
	
	nodeListView.setRadius(nodeListView.getRadius().onlyX() + nodeListView.getNodeListHeight().onlyY());
	nodeListView.setActive(false);
	
	nodeListView.setOnSelectCallback([&] (gui::ListViewElement& e){
		ensure(getComponent().getSelectedResource());
		// Create new node
		if (!e.getSelected().empty()){
			util::Str8 node_name= e.getText(e.getSelected().front());
			addNodeUi(getComponent().addCompositionNodeLogic(node_name), e.getPosition() - getContentElement().getPosition());
			
			e.setSelected(e.getSelected().front(), false);
		}
		
		e.setActive(false);
	});
	
	nodeListView.setOnOutTriggerCallback([&] (gui::ListViewElement& e){
		// Hide node list when pressed outside
		e.setActive(false);
	});
	
	listenForEvent(global::Event::OnNodeUiMove);
	listenForEvent(global::Event::NodeUiAddLinesRequest);
	
}

NodesEcUi::~NodesEcUi(){
	
}

void NodesEcUi::update(){

	if (gUserInput->isTriggered(UserInput::GuiStartLeftDragging) || gUserInput->isTriggered(UserInput::GuiStartMiddleDragging)){
		gui::Element* under_e= gui::gGuiMgr->getCursor().getUnderElement();
		
		bool node_touched= false;
		
		if (under_e){
			for (auto& m : nodeUis){
				if (m.getSuperGuiElement().recursiveHasSubElement(*under_e)){
					node_touched= true;
					break;
				}
			}
		}
	
		if (!node_touched) {
			for (auto& m : nodeUis){
				m.closeFloatings();
			}
		}
		
	}
	
	if (nodeMovedInLastFrame){
		updateNodeVisibility();
		nodeMovedInLastFrame= false;
	}
	
	for (auto& node_ui : nodeUis){
		node_ui.update();
	}
}

void NodesEcUi::onEvent(global::Event& e){
	BaseType::onEvent(e);
	
	switch(e.getType()){
		case global::Event::OnNodeUiMove:
			updateNodeVisibility();
			nodeMovedInLastFrame= true;
		break;
		
		case global::Event::NodeUiAddLinesRequest: {
			nodes::NodeUi* ui= e(global::Event::Object).getPtr<nodes::NodeUi>();
			for (auto& node_ui : nodeUis){
				for (auto& slot_ui : node_ui.getSlots()){
					
					if (!slot_ui->getCompositionNodeSlot()->isInput())
						continue;
					
					for (auto& attached_info : slot_ui->getCompositionNodeSlot()->getAttachedSlotInfos()){
						auto& slot_gui= findSlotGui(attached_info.slot, attached_info.slotSub);
						if (&node_ui != ui && &slot_gui.getOwner().getOwner() != ui) continue;

						slot_ui->attachLine(slot_gui.getOwner(), attached_info.slotSub, attached_info.mySub);
					}
					
				}
			}
			
			getContentElement().recursiveSpatialUpdate();
			
			for (auto& node_ui : nodeUis){
				for (auto& slot_ui : node_ui.getSlots()){
					slot_ui->updateLines();
				}
			}
		}
		
		default:;
	}
}

void NodesEcUi::onResize(){
	BaseType::onResize();
	updateNodeVisibility();
}

void NodesEcUi::addNodeUis(){
	ensure(nodeUis.empty());
	
	nodes::CompositionNodeLogicGroup* group= getComponent().getSelectedResource();
	
	if (!group) return;
	
	for (nodes::CompositionNodeLogicPtr& m : group->getNodes()){
		addNodeUi(*m, util::Coord::P(m->getPosition()));
	}
	
	getContentElement().recursiveSpatialUpdate();
}

void NodesEcUi::updateNodeVisibility(){
	/// @todo Move visibility checks to gui::Element under an option clipSubElements or something
	return; // Buggy so don't use
	
	for (auto& m : nodeUis){
		auto& ui_gui= m.getSuperGuiElement();
		
		bool is_visible= ui_gui.isVisible() && ui_gui.isActive();
		
		if (getContentElement().isPointInside(ui_gui.getPosition())){
			if (!is_visible){
				ui_gui.setVisible();
				ui_gui.setActive();
			}
		}
		else {
			if (is_visible){
				ui_gui.setVisible(false);

			}
			
			if (ui_gui.isActive() && !m.isDragged()){
				ui_gui.setActive(false);
			}
		}
	}
}

void NodesEcUi::addNodeUi(nodes::CompositionNodeLogic& node, const util::Coord& offset){
	nodeUis.pushBack(nodes::NodeUi(node));
	nodeUis.back().create();
	getContentElement().addSubElement(nodeUis.back().getSuperGuiElement());
	nodeUis.back().getSuperGuiElement().setOffset(offset);
}

nodes::NodeSlotGui& NodesEcUi::findSlotGui(const nodes::CompositionNodeSlotHandle& input, nodes::SubSignalType sub){
	nodes::NodeUi* node_ui= nullptr;
	for (auto& m : nodeUis){
		if (&m.getCompositionNodeLogic() == &input->getOwner()){
			node_ui= &m;
			break;
		}
	}
	
	ensure(node_ui);
	
	nodes::NodeSlotUi* node_slot_ui= nullptr;
	for (auto& m : node_ui->getSlots()){
		if (m->getCompositionNodeSlot() == input){
			node_slot_ui= m.get();
			break;
		}
	}
	
	ensure(node_slot_ui);
	
	return node_slot_ui->getSlotGui(sub);
	
}

void NodesEcUi::showNodeList(){
	if (getComponent().getSelectedResource()){
		nodeListView.snapTopMostTo(0); // Scroll to top
		nodeListView.setActive();
		nodeListView.setPosition(gUserInput->getCursorPosition());
	}
}

void NodesEcUi::deleteNode(){
	// Delete previously pressed node
	if (!nodeUis.empty()){
		auto subelements= getContentElement().getSubElements();
		ensure(!subelements.empty());

		for (auto it= nodeUis.begin(); it != nodeUis.end(); ++it){
			if (&it->getSuperGuiElement() == subelements.back()){
				auto& comp_node= it->getCompositionNodeLogic();
				nodeUis.erase(it);
				getComponent().removeCompositionNodeLogic(comp_node);
				break;
			}
		}
	}
}

}}} // ui::game::editor
} // clover