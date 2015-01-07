#include "element_combobox.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {

ComboBoxElement::ComboBoxElement(const util::Coord& pos, const util::Coord& rad)
		: Element(pos, rad.onlyX() + defaultYRad())
		, btn(pos, rad.onlyX() + defaultYRad())
		, listElement(util::Coord(0), util::Coord({rad.x, 0}, rad.getType()) + defaultYRad(), rad)
		, selectedIndex(0)
		, curY(0)
		, stopY(0)
		, autoRadius(true)
		, scrollListener("host", "hoveringListSelection", "scroll", [this] (const util::Vec2d& v){
			curY += v.y;
		}){
	
	elementType= Element::ComboBox;
			
	touchable= false;
	listElement.setApplySuperActivation(false);
	listElement.setActive(false);
	
	if (rad.x == 0){
		setMaxRadius(util::Coord::VSt(1).onlyX() + defaultYRad());
	}
	
	addSubElement(btn);
	addSubElement(listElement);
}

ComboBoxElement::~ComboBoxElement(){
}

void ComboBoxElement::preUpdate(){
	Element::preUpdate();
}

void ComboBoxElement::setActive(bool b){
	Element::setActive(b);
	
	if (!b){
		cancelList();
	}
}

void ComboBoxElement::spatialUpdate(){
	Element::spatialUpdate();
	
	if (!autoRadius){
		btn.setRadius(getRadius()); // Adjust button to element size if size is manually set
	}
	
	if (isActive() && listElement.isActive()){
		// List is visible

		listElement.setScroll(curY);
		
		for (uint32 i=0; i<listElement.size(); ++i){
			listElement.setSelected(i, false);
		}
		uint32 closest= listElement.getClosestZeroOffset();
		listElement.setSelected(closest, true);
		
		if (gUserInput->isTriggered(UserInput::GuiHold) || gUserInput->isTriggered(UserInput::GuiCause)){
			listElement.snapTo(closest);
		}
	}
}

void ComboBoxElement::postUpdate(){
	Element::postUpdate();
	
	util::Coord::Type radtype= listElement.getRadius().getType();
	util::Coord mdif= gUserInput->getCursorDifference().converted(radtype);
	
	if (!isActive()) return;

	if (listElement.isActive()){
		curY += mdif.y/listElement.getNodeListHeight().converted(radtype).y;

		if (gUserInput->isTriggered(UserInput::GuiCause)){
			hoveringListSelectionTagEntry.reset();
			stopY= listElement.getScroll();
			
			auto selected_list= listElement.getSelected();
			if (selected_list.size() == 1){
				selectedIndex= selected_list[0];
				btn.setText(listElement.getText(selectedIndex));

				listElement.setActive(false);
				guiCursor->touchUnlock();
				gUserInput->popCursorLock();
			
				OnSelect(*this);
				OnSelectionChange(*this);
			}
		}
	}
	
	if (btn.isTriggered()){
		listElement.setActive();
		btn.setText("");
		curY= stopY;

		guiCursor->touchLock(listElement);

		hoveringListSelectionTagEntry= ui::hid::TagListEntry("host", "hoveringListSelection");
	}
	else if (gUserInput->isTriggered(UserInput::GuiCancel)){
		cancelList();
	}
}

uint32 ComboBoxElement::append(const util::Str8& text){
	uint32 id= listElement.append(text);
	if (id == 0){
		btn.setText(text);
	}
	return id;
}

void ComboBoxElement::clear(){
	selectedIndex= 0;
	listElement.clear();
	btn.setText("");
}


uint32 ComboBoxElement::getSelectedIndex() const {
	return selectedIndex;
}

void ComboBoxElement::setSelected(SizeType i){
	selectedIndex= i;
	listElement.setSelected(i);
	listElement.snapTo(i);
	btn.setText(listElement.getText(selectedIndex));
	OnSelectionChange(*this);
}

void ComboBoxElement::setSelected(const util::Str8& text){
	auto index= listElement.getIndex(text);
	setSelected(index);
}

util::Str8 ComboBoxElement::getSelectedText() const {
	ensure(selectedIndex >= 0 && !listElement.empty());
	return listElement.getText(selectedIndex);
}

void ComboBoxElement::resetListPosition(){
	stopY= 0;
	selectedIndex= 0;
	if (!listElement.getSubElements().empty())
		btn.setText(listElement.getText(0));
	OnSelectionChange(*this);
}

void ComboBoxElement::setListNodeVerticalRadius(const util::Coord& c){
	listElement.setNodeVerticalRadius(c);
}

void ComboBoxElement::cancelList(){
	listElement.setActive(false);
	if (listElement.size())
		btn.setText(listElement.getText(selectedIndex));
		
	guiCursor->touchUnlock();

	hoveringListSelectionTagEntry.reset();
}

} // gui
} // clover
