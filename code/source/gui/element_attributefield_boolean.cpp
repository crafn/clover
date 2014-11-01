#include "element_attributefield_boolean.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::BooleanAttributeType>::AttributeFieldElement(const resources::AttributeDef& def)
		: AttributeFieldWrapElement(def)
		, checkBox("", util::Coord::VF(0), util::Coord::VF(0.03)){
	addSubElement(checkBox);
	
	checkBox.setOnValueModifyCallback([=] (gui::CheckBoxElement& caller) {
		value= caller.isChecked();
		OnValueModify(*this);
	});
}

AttributeFieldElement<resources::BooleanAttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::BooleanAttributeType>::onValueSet(){
	checkBox.setChecked(value);
	//print(debug::Ch::Gui, debug::Vb::Trivial, "GuiAttriuteField<Boolean>::onValueSet(): value %i", value);
}

} // gui
} // clover