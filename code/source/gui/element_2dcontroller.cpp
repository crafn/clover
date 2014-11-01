#include "element_2dcontroller.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {
	
Controller2dElement::Controller2dElement(const util::Coord& pos, const util::Coord& rad)
		: Element(pos, rad)
		, circleBoundary(false){

	//autoREs.resize(2);
	/*pointRE= &autoREs[1];
	bgRE= &autoREs[0];
			
	pointRE->setModel("Gui2dController_Point");
	pointRE->setEnvLight(0.0);
	pointRE->setColorMul(util::Color{0.5,0.5,0.5,0.5});
	pointRE->setScale(util::Coord::VF(0.01));
	pointRE->setLayer(visual::ModelEntity::Layer::Gui);
	
	bgRE->setModel("Gui2dController_Bg");
	bgRE->setEnvLight(0.3);
	bgRE->setColorMul(util::Color{0.5,0.5,0.5,0.5});
	bgRE->setScale(rad);
	bgRE->setLayer(visual::ModelEntity::Layer::Gui);*/
}

Controller2dElement::~Controller2dElement(){
	
}

void Controller2dElement::postUpdate(){
	Element::postUpdate();

	if (!isActive()) return;
	
	if (guiCursor->getDraggedElement() == this){
		util::Vec2d offset= (gUserInput->getCursorPosition().converted(position.getType()) - position).getValue();
		value = offset * radius.converted(position.getType()).getValue().inversed();
		
		if (circleBoundary){
			if (value.lengthSqr() > 1.0){
				value= value.normalized();
			}
		}

		value.x= util::limited(value.x, -1.0, 1.0);
		value.y= util::limited(value.y, -1.0, 1.0);
	}
	
	updateRE();
}

void Controller2dElement::updateRE(){
	//pointRE->setPosition( position + radius * value);
}

} // gui
} // clover