#include "element_we_icon.hpp"
#include "ui/userinput.hpp"
#include "game/worldentity.hpp"

namespace clover {
namespace gui {

WeIconElement::WeIconElement(game::WeHandle h, const util::Coord& pos, const util::Coord& rad):
		Element(pos, rad),
		handle(h),
		elementVisual(createCustomVisualEntity()),
		selectTriggered(false){
	elementType= WEIcon;
	updateRE();
}

WeIconElement::WeIconElement(WeIconElement&& other):
		Element(std::move(other)),
		handle(std::move(other.handle)),
		elementVisual(other.elementVisual),
		selectTriggered(other.selectTriggered){
	updateRE();
}

WeIconElement::~WeIconElement(){
	destroyVisuals();
}

void WeIconElement::updateRE(){
	if (handle){
		//iconDef.setModel(*handle->getInfo().icon.defaultModel);
		iconDef.setEnvLight(1.0);
		
		elementVisual->getVisualEntity().setDef(iconDef);
		elementVisual->getVisualEntity().setScale(radius.getValue().xyz());
		elementVisual->getVisualEntity().setCoordinateSpace(radius.getType());
	}
}

void WeIconElement::postUpdate(){
	Element::postUpdate();
	selectTriggered= false;
	
	auto temp= guiCursor->getTouchedElement();
	
	if (gUserInput->isTriggered(UserInput::GuiCause) && curBbState == Bb_Touch){
		//print(debug::Ch::WE, debug::Vb::Trivial, "Triggered");
		selectTriggered= true;
	}
}

void WeIconElement::spatialUpdate(){
	Element::spatialUpdate();
	
	if (isActive()){
		elementVisual->setPositionTarget(getPosition());
		elementVisual->setRadiusTarget(getRadius());
	}
}

bool WeIconElement::isSelectTriggered(){
	return selectTriggered;
}

} // gui
} // clover