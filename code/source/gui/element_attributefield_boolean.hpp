#ifndef CLOVER_GUI_ATTRIBUTEFIELD_BOOLEAN_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_BOOLEAN_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_checkbox.hpp"

namespace clover {
namespace gui {

template <>
class AttributeFieldElement<resources::BooleanAttributeType> : public AttributeFieldWrapElement<resources::BooleanAttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldElement();
	
private:
	virtual void onValueSet();
	
	gui::CheckBoxElement checkBox;
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_BOOLEAN_HPP