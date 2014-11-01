#ifndef CLOVER_GUI_ATTRIBUTEFIELD_VECTOR2_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_VECTOR2_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_textfieldarray.hpp"

namespace clover {
namespace gui {

template <>
class AttributeFieldElement<resources::Vector2AttributeType> : public AttributeFieldWrapElement<resources::Vector2AttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldElement();
	
private:
	virtual void onValueSet();
	void onValueChange();
	
	gui::TextFieldArrayElement fieldArray;
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_VECTOR2_HPP