#include "element_attributefield_color.hpp"
#include "resources/cache.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::ColorAttributeType>::AttributeFieldElement(const resources::AttributeDef& def)
		: AttributeFieldWrapElement(def)
		, previewElementEntity(createCustomVisualEntity())
		, colorPickerFloating(util::Coord::VSt(0))
		, colorPicker(util::Coord::VSt(0), colorPickerRadius()){
			
	colorPreviewEntityDef.setModel(global::g_env->resCache->getResource<visual::Model>("guiColorPreview"));

	colorPreviewEntityDef.setEnvLight(1.0);
	previewElementEntity->getVisualEntity().setDef(colorPreviewEntityDef);
	previewElementEntity->getVisualEntity().setScale((radius.getValue()*util::Vec2d{1,0.5}).xyz());
	previewElementEntity->getVisualEntity().setScaleCoordinateSpace(radius.getType());
	
	colorPickerFloating.addSubElement(colorPicker);
	colorPickerFloating.setActive(false);
	colorPickerFloating.setOnOutTriggerCallback([] (gui::FloatingElement& e){
		e.setActive(false);
	});
	
	colorPicker.setOnColorModifyCallback([&] (gui::ColorPickerElement& caller){
		value= caller.getColor();
		colorPreviewEntityDef.setColorMul(value);
		
		//print(debug::Ch::Gui, debug::Vb::Trivial, "util::Color picked: %f, %f, %f, %f", value.r, value.g, value.b, value.a);
	
		OnValueModify(*this);
	});
}

AttributeFieldElement<resources::ColorAttributeType>::~AttributeFieldElement(){
	destroyVisuals();
}

void AttributeFieldElement<resources::ColorAttributeType>::setActive(bool b){
	BaseType::setActive(b);
	
	if (!b)
		colorPickerFloating.setActive(false);
}

void AttributeFieldElement<resources::ColorAttributeType>::preUpdate(){
	BaseType::preUpdate();
}

void AttributeFieldElement<resources::ColorAttributeType>::postUpdate(){
	BaseType::postUpdate();
	
	if (!isActive()) return;
	
	if (curBbState == Bb_Touch && gUserInput->isTriggered(UserInput::GuiCause)){
		colorPickerFloating.setActive();
	}

}

void AttributeFieldElement<resources::ColorAttributeType>::onValueSet(){
	colorPreviewEntityDef.setColorMul(value);
	colorPicker.setColor(value);
	//print(debug::Ch::Gui, debug::Vb::Trivial, "AttributeFieldElement<util::Color>::onValueSet(): %f, %f, %f, %f", value.r, value.g, value.b, value.a);
}

void AttributeFieldElement<resources::ColorAttributeType>::spatialUpdate(){
	BaseType::spatialUpdate();
	
	if (isActive()){
		colorPickerFloating.setOffset(getPosition());
		previewElementEntity->setPositionTarget(getPosition());
	}
}

} // gui
} // clover