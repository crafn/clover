#ifndef CLOVER_GUI_ATTRIBUTEFIELD_VECTOR4_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_VECTOR4_HPP

#include "build.hpp"
#include "element_attributefield.hpp"

namespace clover {
namespace gui {

/// @todo
template <>
class AttributeFieldElement<resources::Vector4AttributeType> : public AttributeFieldWrapElement<resources::Vector4AttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def):AttributeFieldWrapElement(def){}
	virtual ~AttributeFieldElement(){}

private:
	virtual void onValueSet(){}
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_VECTOR4_HPP