#ifndef CLOVER_UI_NODES_NODE_UI_HPP
#define CLOVER_UI_NODES_NODE_UI_HPP

#include "build.hpp"
#include "nodes/compositionnodelogic.hpp"
#include "gui/element.hpp"
#include "gui/element_button.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_textlabel.hpp"
#include "nodeslot_ui.hpp"
#include "nodeslotgroupsettings_ui.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace ui { namespace nodes {

class NodeUi : public global::EventReceiver {
public:

	typedef std::unique_ptr<NodeSlotUi> NodeSlotUiPtr;
	
	NodeUi(CompositionNodeLogic&);
	NodeUi(NodeUi&&);
	NodeUi(const NodeUi&)= delete;
	
	void create();
	
	CompositionNodeLogic& getCompositionNodeLogic(){ return *logic; }
	
	gui::Element& getSuperGuiElement(){ return bgPanel; }
	bool isDragged() const { return titlePanel.isDragged(); }
	
	NodeSlotUi& getSlotUi(const CompositionNodeSlot& slot);
	const util::DynArray<NodeSlotUiPtr>& getSlots() const { return slots; }
	util::DynArray<NodeSlotUiPtr>& getSlots(){ return slots; }
	
	virtual void onEvent(global::Event& e) override;
	
	void closeFloatings();
	
	void update();
	
private:
	void createSlots();
	void bind();
	util::Coord titlePanelYRad() const { return util::Coord::P({0, 10}); }
	util::Coord minGroupButtonRad() const { return util::Coord::P({10, 5}); }
	void adjustRadius();
	void closeOtherSubSlots(const NodeSlotUi& ui);
	
	void onMove(gui::Element& e);

	NodeSlotUi& createSlot(const SlotIdentifier& slot_id);
	void destroySlot(const SlotIdentifier& slot_id);
	
	// Change visuals
	void updateErrorState();
	
	CompositionNodeLogic* logic;
	
	util::CbListener<util::OnChangeCb> nodeTypeChangeListener;
	
	gui::PanelElement bgPanel;
	
	gui::LinearLayoutElement contentLayout;
		gui::PanelElement titlePanel;
		gui::TextLabelElement titleLabel;
	
	gui::LinearLayoutElement slotLayout;
		gui::LinearLayoutElement inputSlotLayout;
		gui::ButtonElement inputSlotGroupButton;
		gui::LinearLayoutElement outputSlotLayout;
		gui::ButtonElement outputSlotGroupButton;
	
	NodeSlotGroupSettingsUi slotGroupSettingsUi;
	
	util::DynArray<NodeSlotUiPtr> slots;
};

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_NODE_UI_HPP