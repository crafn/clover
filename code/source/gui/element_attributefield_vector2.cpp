#include "element_attributefield_vector2.hpp"
#include "element_attributefield_scalar.hpp"

namespace clover {
namespace gui {

AttributeFieldElement<resources::Vector2AttributeType>::AttributeFieldElement(const resources::AttributeDef& def):
	AttributeFieldWrapElement(def),
	fieldArray(2, util::Coord::VF(0), radius){
	
	addSubElement(fieldArray);
	resizeElements.pushBack(&fieldArray);
	
	for (auto& m : fieldArray){
		m.setNumeric();
	
		m.setOnEditingStateChangeCallback([=] (gui::TextFieldElement& caller){
			if (!caller.isEditing())
				OnValueModify(*this);
		});
	
	}
	
	fieldArray.setOnTextModifyCallback([&] (gui::TextFieldArrayElement& caller) {
		value= util::Vec2d { caller[0].getValueAs(value.x), caller[1].getValueAs(value.y) };
		onValueChange();
	});
}

AttributeFieldElement<resources::Vector2AttributeType>::~AttributeFieldElement(){
}

void AttributeFieldElement<resources::Vector2AttributeType>::onValueSet(){
	fieldArray[0].setValueAs(value.x);
	fieldArray[1].setValueAs(value.y);
	
	onValueChange();
	//print(debug::Ch::Gui, debug::Vb::Trivial, "AttributeFieldElement<Vector2>::onValueSet(): value %f, %f", value.x, value.y);
}

void AttributeFieldElement<resources::Vector2AttributeType>::onValueChange(){

}

} // gui
} // clover