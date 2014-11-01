#include "element_attributefield_resource.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::ResourceAttributeType>::AttributeFieldElement(const resources::AttributeDef& def):
	AttributeFieldWrapElement(def),
	textField(TextFieldElement::Type::Dev){
	
	addSubElement(textField);
	textField.setStretchable();
	resizeElements.pushBack(&textField);

	textField.setOnEditingStateChangeCallback([=] (gui::TextFieldElement& caller){
		if (!caller.isEditing())
			OnValueModify(*this);
	});

	textField.setOnTextModifyCallback([=] (gui::TextFieldElement& caller) {
		value= caller.getText();
		onValueChange();
	});
}

AttributeFieldElement<resources::ResourceAttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::ResourceAttributeType>::onValueSet(){
	textField.setText(value);
	onValueChange();
	//print(debug::Ch::Gui, debug::Vb::Trivial, "GuiAttriuteField<Resource>::onValueSet(): value %s", value.cStr());
}

void AttributeFieldElement<resources::ResourceAttributeType>::onValueChange(){

	textField.setTextColorMul(util::Color{0.8, 0.9, 0.3, 1.0});
}

} // gui
} // clover