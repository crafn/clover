#include "element_contextmenu.hpp"

namespace clover {
namespace gui {
	
ContextMenuElement::ContextMenuElement():
	Base(){
	setActive(false);
	addSubElement(floating);
	floating.addSubElement(listView);
	
	bind();
}

void ContextMenuElement::bind(){
	floating.setOnOutTriggerCallback([&] (gui::FloatingElement&) {
		setActive(false);
	});
	
	listView.setOnSelectCallback([&] (gui::ListViewElement&) {
		OnSelect(*this);
		setActive(false);
	});
}

void ContextMenuElement::show(const util::Coord& pos_hint){
	listView.clearSelection();
	floating.show(pos_hint);
	setActive();
}

auto ContextMenuElement::append(const util::Str8& text, NodeType type, NodeState state) -> NodeId {
	auto id= listView.append(text, type, state);
	listView.snapTopMostTo(0);
	floating.spatialUpdate();
	return id;
}

auto ContextMenuElement::getSelected() const -> util::DynArray<NodeId> {
	return listView.getSelected();
}

util::Str8 ContextMenuElement::getText(NodeId id) const {
	return listView.getText(id);
}

void ContextMenuElement::spatialUpdate(){
	Base::spatialUpdate();
	setRadius(floating.getRadius());
}

} // gui
} // clover