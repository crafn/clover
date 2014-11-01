#ifndef CLOVER_UI_EDITOR_EC_UI_NODES_HPP
#define CLOVER_UI_EDITOR_EC_UI_NODES_HPP

#include "build.hpp"
#include "ec_ui_resource.hpp"
#include "game/editor/components/ec_nodes.hpp"
#include "gui/element_listview.hpp"
#include "ui/hid/actionlistener.hpp"
#include "ui/nodes/node_ui.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

template <>
struct EditorComponentUiTraits<NodesEc> {
	static util::Coord defaultRadius(){ return util::Coord::VSt({1.0,1.0}); }
};

class NodesEcUi : public BaseResourceEcUi<NodesEc> {
public:
	typedef BaseResourceEcUi<NodesEc> BaseType;
	
	NodesEcUi(NodesEc&);
	virtual ~NodesEcUi();
	
	virtual void update() override;
	
	virtual void onEvent(global::Event& e) override;
	
protected:
	virtual void onResize();
	
private:
	void addNodeUis();
	void updateNodeVisibility();
	void addNodeUi(nodes::CompositionNodeLogic& node, const util::Coord& offset);
	
	ui::nodes::NodeSlotGui& findSlotGui(const nodes::CompositionNodeSlotHandle& input, nodes::SubSignalType sub);
	
	void showNodeList();
	void deleteNode();
	
	util::LinkedList<ui::nodes::NodeUi> nodeUis;
	gui::ListViewElement nodeListView;
	// Ugly hack for hiding elements outside
	bool nodeMovedInLastFrame;
	
	ui::hid::ActionListener<> showNodeListListener;
	ui::hid::ActionListener<> deleteNodeListener;
	
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EC_UI_NODES_HPP