#ifndef CLOVER_GUI_ATTRIBUTEFIELD_BASESHAPE_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_BASESHAPE_HPP

#include "build.hpp"
#include "element_attributefield.hpp"

namespace clover {
namespace gui {

/// Editing in blender
template <>
class AttributeFieldElement<resources::BaseShapeAttributeType> : public AttributeFieldWrapElement<resources::BaseShapeAttributeType> {
public:
	AttributeFieldElement(const resources::AttributeDef& def):AttributeFieldWrapElement(def){}
	virtual ~AttributeFieldElement(){}

private:
	virtual void onValueSet(){}
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_BASESHAPE_HPP