#include "element_attributefield_resourcepair.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::ResourcePairAttributeType>::AttributeFieldElement(const resources::AttributeDef& def):
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
		auto str= caller.getText();
		auto parts= str.splitted(' ');
		
		if (parts.size() > 0)
			value.first= parts[0];
		else value.first= "";
		
		if (parts.size() > 1)
			value.second= parts[1];
		else value.second= "";
	});
}

AttributeFieldElement<resources::ResourcePairAttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::ResourcePairAttributeType>::onValueSet(){
	textField.setText(value.first + " " + value.second);
}

} // gui
} // clover