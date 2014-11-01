#ifndef CLOVER_GUI_ELEMENT_CONTEXTMENU_HPP
#define CLOVER_GUI_ELEMENT_CONTEXTMENU_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_floating.hpp"
#include "element_listview.hpp"
#include "element_button.hpp"

namespace clover {
namespace gui {

/// Context menu; list which pops up and stuff can be clicked
/// Hidden by default
class ContextMenuElement : public Element {
public:
	DECLARE_GUI_ELEMENT(ContextMenuElement)
	using Base= Element;
	
	using NodeType= ListViewElement::NodeType;
	using NodeState= ListViewElement::NodeState;
	using Node= ListViewElement::Node;
	using NodeId= ListViewElement::NodeId;
	
	ContextMenuElement();
	ContextMenuElement(ContextMenuElement&&)= delete; /// @todo
	ContextMenuElement& operator=(ContextMenuElement&&)= delete; /// @todo
	virtual ~ContextMenuElement(){}
	
	void show(const util::Coord& pos_hint);
	
	NodeId append(const util::Str8& text, NodeType type= NodeType::Text, NodeState state= NodeState::Enabled);
	util::DynArray<NodeId> getSelected() const;
	util::Str8 getText(NodeId id) const;
	
	GUI_CALLBACK(OnSelect)
	
protected:
	void bind();
	virtual void spatialUpdate() override;
	
private:
	FloatingElement floating;
	ListViewElement listView;
};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENT_CONTEXTMENU_HPP