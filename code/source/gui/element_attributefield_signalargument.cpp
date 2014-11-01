#include "element_attributefield_signalargument.hpp"
#include "nodes/signaltypetraits.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::SignalArgumentAttributeType>::AttributeFieldElement(const resources::AttributeDef& def)
	: AttributeFieldWrapElement(def)
	, layout(LinearLayoutElement::Horizontal)
	, nameField(TextFieldElement::Type::Dev){
	
	addSubElement(layout);

	resizeElements.pushBack(&layout);
	
	for (const auto& m : nodes::RuntimeSignalTypeTraits::signalTypes())
		typeCombo.append(nodes::RuntimeSignalTypeTraits::enumString(m));
		
	nameField.setCharacterWidth(15);
	
	layout.addSubElement(typeCombo);
	layout.addSubElement(nameField);
	
	layout.setMargin();
	util::Coord rad(nameField.getMinRadius().onlyX() + typeCombo.getMinRadius() + util::Coord::P(50));

	layout.setMinRadius(rad);
	layout.setRadius(rad);
	
	typeCombo.setOnSelectCallback([=] (gui::Element& e){
		value.signalType= static_cast<nodes::SignalType>(typeCombo.getSelectedIndex());
		OnValueModify(*this);
	});
	
	nameField.setOnEditingStateChangeCallback([=] (gui::TextFieldElement& e){
		if (!nameField.isEditing()){
			value.name= nameField.getText();
			OnValueModify(*this);
		}
	});
	
}

void AttributeFieldElement<resources::SignalArgumentAttributeType>::onValueSet(){
	typeCombo.setSelected(static_cast<uint32>(value.signalType));
	nameField.setText(value.name);
}

} // gui
} // clover