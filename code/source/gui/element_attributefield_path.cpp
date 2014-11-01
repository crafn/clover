#include "element_attributefield_path.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::PathAttributeType>::AttributeFieldElement(const resources::AttributeDef& def):
	AttributeFieldWrapElement(def),
	textField(TextFieldElement::Type::Dev){
	
	addSubElement(textField);
	textField.setStretchable();
	resizeElements.pushBack(&textField);

	textField.setOnTextModifyCallback([=] (gui::TextFieldElement& caller) {
		value.setValue(caller.getText());
		onValueChange();
		OnValueModify(*this);
	});
}

AttributeFieldElement<resources::PathAttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::PathAttributeType>::onValueSet(){
	textField.setText(value.value());
	onValueChange();
	//print(debug::Ch::Gui, debug::Vb::Trivial, "GuiAttriuteField<Path>::onValueSet(): value %s", value.value().cStr());
}

void AttributeFieldElement<resources::PathAttributeType>::onValueChange(){
	bool is_pointing_to_file= value.isValid();
	
	if (is_pointing_to_file)
		textField.setTextColorMul(util::Color{0.6, 0.6, 0.6, 1.0});
	else
		textField.setTextColorMul(util::Color{1.0, 0.0, 0.0, 1.0});
}

} // gui
} // clover