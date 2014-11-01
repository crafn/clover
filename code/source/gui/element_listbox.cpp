#include "element_listbox.hpp"

namespace clover {
namespace gui {

ListBoxElement::ListBoxElement(const util::Coord& pos, const util::Coord& radius):
		Element(pos, radius),
		list(util::Coord::VF(0), radius, util::Coord::VF({0,0.025})){
		
	elementType= Element::ListBox;
		
	list.setWrapFade(false);
		
	addSubElement(list);
	
	list.setOnSelectCallback([=] (gui::ListViewElement&) {
		OnSelect(*this);
	});
}

ListBoxElement::~ListBoxElement(){
}

auto ListBoxElement::getSelected() const -> util::DynArray<NodeId> {
	return list.getSelected();
}

util::Str8 ListBoxElement::getText(NodeId node) const {
	return list.getText(node);
}

util::Coord ListBoxElement::getNodeRadius() const {
	return util::Coord::VF({0,0.025}) + util::Coord({radius.x, 0}, radius.getType());
}

auto ListBoxElement::append(const util::Str8& text, NodeType type, NodeState state) -> NodeId{
	return list.append(text, type, state);
}

void ListBoxElement::clear(){
	list.clear();
}

void ListBoxElement::scrollToBegin(){
	if (!list.empty())
		list.snapTopMostTo(0);
}

void ListBoxElement::setRadius(const util::Coord& r){
	Element::setRadius(r);
	list.setRadius(r);
}

void ListBoxElement::setMaxRadius(const util::Coord& r){
	Element::setMaxRadius(r);
	list.setMaxRadius(r);
}

void ListBoxElement::setMinRadius(const util::Coord& r){
	Element::setMinRadius(r);
	list.setMinRadius(r);
}

} // gui
} // clover