#include "element_checkbox.hpp"

namespace clover {
namespace gui {

CheckBoxElement::CheckBoxElement(const util::Str8& label_, const util::Coord& pos, const util::Coord& box_rad)
		: Element(pos, util::Coord::VSt(0))
		, checked(false)
		, boxRadius(box_rad)
		, btn(pos,util::Coord::VSt(0))
		, label(label_, pos)
		, checkVisual(createVisualEntity(ElementVisualEntity::Type::Panel)){
	elementType= CheckBox;

	checkVisual->setOnTopOfElement();
	boxRadius.setRelative();

	addSubElement(btn);
	addSubElement(label);
	
	checkVisual->setRadiusTarget(box_rad*0.7);
	
	updateOffsets();
}

CheckBoxElement::~CheckBoxElement(){
}

void CheckBoxElement::preUpdate(){
	Element::preUpdate();
}

bool CheckBoxElement::isToggled() const {
	if (isActive() && btn.isTriggered()) return true;
	return false;
}

void CheckBoxElement::postUpdate(){
	Element::postUpdate();
	
	if (isActive() && btn.isTriggered()){
		setChecked(!checked);
		OnValueModify(*this);
	}
}

void CheckBoxElement::updateOffsets(){
	util::Coord::Type type= boxRadius.getType();
	
	util::Coord rad(type);
	rad.setRelative();
	util::Coord label_rad=	label.getRadius().converted(type);
	label_rad.setRelative();
	rad.x= label_rad.x + boxRadius.x;
	rad.y= label_rad.y;
	if (rad.y < boxRadius.y) rad.y= boxRadius.y;
	
	setRadius(rad);
	setMinRadius(rad);
	setMaxRadius(rad);
	
	btn.setRadius(rad);
	
	label.setOffset(util::Coord({boxRadius.x, 0}, type));
	
	checkVisual->setPositionTarget(position - util::Coord({radius.x - boxRadius.x, 0}, type));
}

void CheckBoxElement::setChecked(bool b){
	if (checked == b) return;
	
	checked= b;
	
	if (checked){
		checkVisual->setHighlightTarget(true);
	}
	else {
		checkVisual->setHighlightTarget(false);
	}
}

void CheckBoxElement::spatialUpdate(){
	Element::spatialUpdate();
	updateOffsets();
}

} // gui
} // clover