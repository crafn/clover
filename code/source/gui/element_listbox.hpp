#ifndef CLOVER_GUI_LISTBOX_HPP
#define CLOVER_GUI_LISTBOX_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_textlabel.hpp"
#include "element_listview.hpp"

namespace clover {
namespace gui {

class ListBoxElement : public Element {
public:
	DECLARE_GUI_ELEMENT(ListBoxElement)
	
	using NodeState= ListViewElement::NodeState;
	using NodeType= ListViewElement::NodeType;
	using NodeId= ListViewElement::NodeId;
	
	ListBoxElement(const util::Coord& pos, const util::Coord& radius);
	ListBoxElement(const ListBoxElement&)= delete;
	ListBoxElement(ListBoxElement&& other)= delete;
	ListBoxElement& operator=(ListBoxElement&& other)= delete;
	ListBoxElement& operator=(ListBoxElement& other)= delete;
	virtual ~ListBoxElement();
	
	util::DynArray<NodeId> getSelected() const;
	util::Str8 getText(NodeId node) const;

	util::Coord getNodeRadius() const;

	/// Returns an index of node added
	NodeId append(const util::Str8& text, NodeType type= NodeType::Text, NodeState state= NodeState::Enabled);
	
	bool hasNode(const util::Str8& text) const { return list.hasNode(text); }
	void setSelected(const util::Str8& text, bool s= true){ list.setSelected(text, s); }
	
	void clear();
	
	void scrollToBegin();
	
	virtual void setRadius(const util::Coord&);
	virtual void setMaxRadius(const util::Coord&);
	virtual void setMinRadius(const util::Coord&);
	
	/// When a node of the list is selected
	GUI_CALLBACK(OnSelect)
	
private:
	gui::ListViewElement list;
};

} // gui
} // clover

#endif // CLOVER_GUI_LISTBOX_HPP