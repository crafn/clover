#include "element_attributefield_scalar.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::RealAttributeType>::AttributeFieldElement(const resources::AttributeDef& def):
	AttributeFieldWrapElement(def),
	textField(TextFieldElement::Type::Dev){
	
	addSubElement(textField);
	textField.setStretchable();
	
	textField.setNumeric();
	
	textField.setOnEditingStateChangeCallback([=] (gui::TextFieldElement& caller){
		if (!caller.isEditing())
			OnValueModify(*this);
	});
	
	textField.setOnTextModifyCallback([=] (gui::TextFieldElement& caller) {
		value= caller.getValueAs<real64>(value);
		onValueChange();
	});
}	

AttributeFieldElement<resources::RealAttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::RealAttributeType>::onValueSet(){
	textField.setValueAs(value);
	onValueChange();
	//print(debug::Ch::Gui, debug::Vb::Trivial, "GuiAttriuteField<Real>::onValueSet(): value %f", value);
}

void AttributeFieldElement<resources::RealAttributeType>::onValueChange(){
	if (value < getAttributeDefImpl().minValue || value > getAttributeDefImpl().maxValue){
		textField.setTextColorMul(util::Color{1,0,0,1});
	}
	else {
		textField.setTextColorMul(util::Color{0.5,0.3,0.8,1});
	}
}

} // gui
} // clover