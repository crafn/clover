#include "element_floating.hpp"
#include "ui/userinput.hpp"
#include "hardware/mouse.hpp"

namespace clover {
namespace gui {

FloatingElement::FloatingElement(const util::Coord& pos):
	Base(pos, util::Coord::P(0)){
	touchable= false;
	depthOffset= 1000;
}

FloatingElement::~FloatingElement(){
}

void FloatingElement::show(const util::Coord& pos_hint){
	setActive();
	setPosition(getLimitedPosition(pos_hint + getRadius().onlyX() - getRadius().onlyY()));
}

void FloatingElement::setOffset(const util::Coord& o){
	util::Coord super_pos= getPosition() - getOffset();
	Base::setOffset(getLimitedPosition(super_pos + o) - super_pos);
}

void FloatingElement::postUpdate(){
	if (isActive()){
		bool on_out= false;
		
		if ((	hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_LEFT) ||
				hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_RIGHT) ||
				hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_MIDDLE)) && 
			!recursiveIsTouched())
				on_out= true;
				
		if (gUserInput->isTriggered(UserInput::GuiConfirm))
			on_out= true;
		
		if (on_out)
			OnOutTrigger(*this);
	
	}
	
	Base::postUpdate();
}

void FloatingElement::spatialUpdate(){
	Base::spatialUpdate();

	if (!subElements.empty()){
		Element* e= subElements.front();
		setMinRadius(e->getMinRadius());
		setMaxRadius(e->getMaxRadius());
		setRadius(e->getRadius());
		setOffset(getOffset()); // Make sure we're on the screen
	}
}

util::Coord FloatingElement::getLimitedPosition(const util::Coord& pos_){
	util::Coord rad= radius.converted(util::Coord::View_Stretch);
	util::Coord pos= pos_.converted(util::Coord::View_Stretch);
	
	if (pos.x + rad.x > 1.0){
		pos.x= 1.0 - rad.x;
	}
	else if (pos.x - rad.x < -1.0){
		pos.x= -1.0 + rad.x;
	}
	
	if (pos.y + rad.y > 1.0){
		pos.y= 1.0 - rad.y;
	}
	else if (pos.y - rad.y < -1.0){
		pos.y= -1.0 + rad.y;
	}
	
	return pos.converted(pos_.getType());
}

} // gui
} // clover