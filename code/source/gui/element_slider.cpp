#include "element_slider.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {

SliderElement::SliderElement(Type t, const util::Coord& pos, const util::Coord& rad, real32 rel_handle_size):
		Element(pos, rad),
		type(t),
		handleVisual(createVisualEntity(ElementVisualEntity::Type::Panel)),
		railVisual(createVisualEntity(ElementVisualEntity::Type::Panel)),
		relHandleSize(rel_handle_size),
		value(0),
		stepCount(0),
		fixedValue(0),
		valueChanged(false){
	elementType= Slider;
			
	util::Coord real_rad= rad;
	real_rad[getSecComponent()] = util::Coord(0.025, real_rad.getType())[getSecComponent()];
	setRadius(real_rad);
	setMinRadius(real_rad);
	setMaxRadius(real_rad);
	
	util::Coord rail_rad= real_rad;
	rail_rad[getSecComponent()]= 0.01;
	
	railVisual->setRadiusTarget(rail_rad);
	
	util::Coord handle_rad(real_rad.getType());
	handle_rad[getPrimComponent()]= real_rad[getPrimComponent()]*rel_handle_size;
	handle_rad[getSecComponent()]=0.025;
	
	handleVisual->setRadiusTarget(handle_rad);
}

SliderElement::~SliderElement(){
}

void SliderElement::setValue(real32 value_){
	value= value_;
	updateHandle();
	valueChanged= true;
	OnValueChange(*this);
}

void SliderElement::setFixedValue(uint32 v){
	value= (real64)v/(stepCount-1);
	fixedValue= v;
	
	updateHandle();
	valueChanged= true;
	OnValueChange(*this);
}

void SliderElement::updateHandle(){
	util::Coord pos(position.getType());
	pos = radius;
	pos[getSecComponent()]= 0;
	
	handleVisual->setPositionTarget(position + pos*(value-0.5)*2*(1 - relHandleSize));
}

void SliderElement::preUpdate(){
	
	Element::preUpdate();
	
	updateHandle();
}

void SliderElement::postUpdate(){
	Element::postUpdate();
	
	valueChanged= false;
	
	if (isActive()){
	
		if (guiCursor->getDraggedElement() == this){
			real32 old_value= value;
			
			// Päivitetään value
			value= (gUserInput->getCursorPosition().converted(position.getType())[getPrimComponent()]
					- position[getPrimComponent()])/radius.converted(position.getType())[getPrimComponent()]*0.5/(1-relHandleSize) + 0.5;
			
			value= util::limited(value, 0.0f, 1.0f);
			
			if (stepCount > 1){
				// Snapataan
				
				int32 closest=-1;
				real64 mindist=0;
				for (int32 i=0; i<(int32)stepCount; ++i){
					
					real64 dist= util::abs(value - (real64)i/(stepCount-1));
					
					if (closest == -1 || mindist > dist){
						mindist= dist;
						closest= i;
					} 
				}
				
				fixedValue= (uint32)closest;
				
				value= (real64)closest/(stepCount-1);
			}
			if (value != old_value){
				valueChanged= true;
				OnValueChange(*this);
				OnValueModify(*this);
			}
			updateHandle();
		}
	}
}

void SliderElement::spatialUpdate(){
	BaseType::spatialUpdate();
	
	railVisual->setPositionTarget(getPosition());
	
	updateHandle();
}

} // gui
} // clover