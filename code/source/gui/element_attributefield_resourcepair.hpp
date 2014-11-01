#ifndef CLOVER_GUI_ATTRIBUTEFIELD_RESOURCEPAIR_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_RESOURCEPAIR_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_textfield.hpp"

namespace clover {
namespace gui {

template <>
class AttributeFieldElement<resources::ResourcePairAttributeType> : public AttributeFieldWrapElement<resources::ResourcePairAttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldElement();

private:
	virtual void onValueSet();

	gui::TextFieldElement textField;
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_RESOURCEPAIR_HPP