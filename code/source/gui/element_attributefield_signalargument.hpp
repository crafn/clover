#ifndef CLOVER_GUI_ATTRIBUTEFIELD_SIGNALARGUMENT_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_SIGNALARGUMENT_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_combobox.hpp"
#include "element_layout_linear.hpp"
#include "element_textfield.hpp"

namespace clover {
namespace gui {

template <>
class AttributeFieldElement<resources::SignalArgumentAttributeType> : public AttributeFieldWrapElement<resources::SignalArgumentAttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldElement(){}

private:
	virtual void onValueSet() override;
	
	LinearLayoutElement layout;
	ComboBoxElement typeCombo;
	TextFieldElement nameField;
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_SIGNALARGUMENT_HPP