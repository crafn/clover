#include "element_attributefield_string.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::StringAttributeType>::AttributeFieldElement(const resources::AttributeDef& def):
	AttributeFieldWrapElement(def),
	textField(TextFieldElement::Type::Dev){
	
	addSubElement(textField);
	textField.setStretchable();
	resizeElements.pushBack(&textField);

	textField.setTextColorMul(util::Color{0.4, 0.8, 0.6, 1.0});

	textField.setOnEditingStateChangeCallback([=] (gui::TextFieldElement& caller){
		if (!caller.isEditing())
			OnValueModify(*this);
	});

	textField.setOnTextModifyCallback([=] (gui::TextFieldElement& caller) {
		value= caller.getText();
	});
}

AttributeFieldElement<resources::StringAttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::StringAttributeType>::onValueSet(){
	textField.setText(value);
}

} // gui
} // clover