#include "element_button.hpp"
#include "audio/soundinstancehandle.hpp"
#include "util/time.hpp"
#include "util/coord.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {

void ButtonElement::init(){
	elementType= Element::Button;
	buttonVisual= createVisualEntity(ElementVisualEntity::Type::Panel);
	textVisual= createTextVisualEntity(TextElementVisualEntity::Type::Dev);

	drawRadiusMul= 1.0;
	
	spatialUpdate();
}

ButtonElement::ButtonElement(const util::Coord& pos, const util::Coord& rad)
		: Element(pos,rad){
	init();
}

ButtonElement::ButtonElement(const util::Str8& text, const util::Coord& pos, const util::Coord& rad)
		: Element(pos,rad){
	init();
	setText(text);
}

ButtonElement::~ButtonElement(){
}

void ButtonElement::setText(const util::Str8& t){
	text= t;
	textVisual->setText(t);
	
	setMinRadius(textVisual->getRadius() + util::Coord::P({2,2}));
	setMaxRadius(getMinRadius()*1.5);
}

const util::Str8& ButtonElement::getText() const {
	return text;
}

void ButtonElement::postUpdate(){
	Element::postUpdate();
	
	if (!enabled){
		drawRadiusMul= 1.0;
		buttonVisual->setHighlightTarget(false);
		return;
	}
	
	if (prevBbState != curBbState){
		if (curBbState == Bb_Touch){

			if (gUserInput->isTriggered(UserInput::GuiCause)){
				audioSource.playSound("guiButton_press");
			}
			else if (prevBbState == Bb_None && guiCursor->getDraggedElement() == nullptr){
				audioSource.playSound("guiButton_hover");
			}

		}
	}
	
	drawRadiusMul= 1.0;
	if (curBbState == Bb_Touch && gUserInput->isTriggered(UserInput::GuiHold))
		drawRadiusMul= 0.9;

	if (curBbState == Bb_Touch)
		buttonVisual->setHighlightTarget(true);
	else
		buttonVisual->setHighlightTarget(false);

}

void ButtonElement::spatialUpdate(){
	Element::spatialUpdate();
	
	buttonVisual->setPositionTarget(position);
	buttonVisual->setRadiusTarget(radius*drawRadiusMul);
	
	textVisual->setPositionTarget(position);
}

} // gui
} // clover