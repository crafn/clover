#ifndef CLOVER_UI_NODES_NODESLOT_UI_HPP
#define CLOVER_UI_NODES_NODESLOT_UI_HPP

#include "build.hpp"
#include "basesignalvalue_ui.hpp"
#include "nodes/compositionnodelogic.hpp"
#include "nodes/compositionnodeslothandle.hpp"
#include "gui/element.hpp"
#include "gui/element_floating.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_textlabel.hpp"
#include "nodeslot_ui.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace ui { namespace nodes {
using namespace clover::nodes;

class NodeSlotUi;

class NodeSlotGui : public gui::Element, public global::EventReceiver {
public:
	typedef gui::Element BaseType;

	static gui::TextLabelElement addSignalTypeLabel(SignalType s, gui::Element& e);

	NodeSlotGui(NodeSlotUi& owner, SubSignalType sub_type= SubSignalType::None);
	NodeSlotGui(NodeSlotGui&&)= delete;
	virtual ~NodeSlotGui();
	
	void setOwner(NodeSlotUi& o){ owner= &o; }
	NodeSlotUi& getOwner(){ ensure(owner); return *owner; }
	
	/// Can be called only for input
	void attachLine(NodeSlotGui& other, bool proxy= false);
	void detachLine(NodeSlotGui& other);
	void detachLines(bool only_proxy= false);
	void detachProxyLines();
	NodeSlotGui* getAttached();
	SubSignalType getSubSignalType(){ return subType; }
	BaseSignalValueUi* getSignalValueUi(){ return signalValueUi.get(); }
	
	/// Always true if SubSignalType == None
	/// False if subslot and it's not shown at the moment
	bool isShown() const { return isActive() || subType == SubSignalType::None; }
	
	void updateLines();
	
	virtual void setRadius(const util::Coord& r);
	
	virtual void spatialUpdate() override;
	virtual void setActive(bool b= true) override;
	void updateLabel();
	void adjustRadius();

	virtual void onEvent(global::Event& e) override;
	
	
	void addArrivingLineSlot(NodeSlotGui& slot);
	void removeArrivingLineSlot(NodeSlotGui& slot);
	
private:
	
	NodeSlotUi* owner;
	SubSignalType subType;
	
	gui::LinearLayoutElement bgLayout;
	gui::PanelElement bgPanel;
	gui::LinearLayoutElement contentLayout;
	gui::TextLabelElement label;
	gui::TextLabelElement signalTypeLabel;
	
	class Line {
	public:
		Line(NodeSlotGui& start, NodeSlotGui& end, gui::ElementVisualEntity& entity, bool proxy);
		Line(const Line&)= delete;
		Line(Line&&)= delete;
		~Line();
		
		void create();
		
		void generate();
		
		void spatialUpdate();
		gui::ElementVisualEntity& getElementVisualEntity(){ ensure(elementVisualEntity); return *elementVisualEntity; }
		
		NodeSlotGui& getStartSlot(){ ensure(startSlot); return *startSlot; }
		NodeSlotGui& getEndSlot(){ ensure(endSlot); return *endSlot; }
		
		bool isProxy() const { return proxy; }
		

	private:
		void generateMesh(util::Vec2f start, util::Vec2f end);
	
		NodeSlotGui* startSlot;
		NodeSlotGui* endSlot;
		
		/// If startSlot is disabled (subSlot) then start of the line is moved elsewhere
		bool startIsMoved;
		
		visual::TriMesh mesh;
		visual::Model model;
		visual::ModelEntityDef modelEntityDef;
		gui::ElementVisualEntity* elementVisualEntity;
		
		util::Vec2f lastStart, lastEnd;
		
		bool proxy;
	};
	
	// Lines from this to start (= output)
	util::LinkedList<Line> lines;
	
	// Connected nodes which have line to this slot
	// Only output-slots should have these
	util::DynArray<NodeSlotGui*> arrivingLineSlots;
	
	// Default value
	std::unique_ptr<BaseSignalValueUi> signalValueUi;
};

class NodeUi;
class NodeSlotUi : public global::EventReceiver {
public:
	
	typedef gui::Element BaseType;
	
	NodeSlotUi(const NodeUi& owner, const CompositionNodeSlotHandle& slot);
	NodeSlotUi(NodeSlotUi&& other)= delete;
	NodeSlotUi(const NodeSlotUi&)= delete;
	virtual ~NodeSlotUi();
	
	NodeSlotUi& operator=(NodeSlotUi&&)= delete;
	NodeSlotUi& operator=(const NodeSlotUi&)= delete;

	const NodeUi& getOwner() const { ensure(owner); return *owner; }
	CompositionNodeSlotHandle getCompositionNodeSlot() const { return compositionNodeSlot; }
	NodeSlotGui& getSuperGuiElement(){ return slotGui; }
	
	void setHorizontalColumnRadius(const util::Coord& r);
	void resetHorizontalColumnRadius();
	
	void showSubSlots(bool b=true);
	bool isSubSlotsShowing() const { return subSlotFloating.isActive(); }
	
	void spatialUpdate();
	
	virtual void onEvent(global::Event& e) override;
	
	/// Only for input-slots
	void attachLine(NodeSlotUi& output, SubSignalType from_sub, SubSignalType to_sub);
	void updateLines();
	
	void update(){}
	
	NodeSlotGui& getSlotGui(SubSignalType s);
	
	typedef std::function<void ()> CallbackType;
	
	/// Not called in showSubSlots, but when slot itself is showing its subSlots
	void setOnSubSlotsShowCallback(const CallbackType& c){ onSubSlotsShow= c; }

private:
	void bind();
	void onDragging(gui::Element& e);
	void onDraggingStop(gui::Element& e);
	
	const NodeUi* owner;
	CompositionNodeSlotHandle compositionNodeSlot;

	NodeSlotGui slotGui;
	
	// SubSlots
	gui::FloatingElement subSlotFloating;
	gui::LinearLayoutElement subSlotLayout;
	
	typedef std::unique_ptr<NodeSlotGui> NodeSlotGuiPtr;
	util::DynArray<NodeSlotGuiPtr> subSlots;
	
	CallbackType onSubSlotsShow;
};

}} // ui::nodes
} // clover

#endif // CLOVER_UI_NODES_NODESLOT_UI_HPP