#ifndef CLOVER_GUI_ATTRIBUTEFIELD_SCALAR_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_SCALAR_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_textfield.hpp"

namespace clover {
namespace gui {

template <>
class AttributeFieldElement<resources::RealAttributeType> : public AttributeFieldWrapElement<resources::RealAttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldElement();

private:
	virtual void onValueSet();
	void onValueChange();
	
	gui::TextFieldElement textField;
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_SCALAR_HPP