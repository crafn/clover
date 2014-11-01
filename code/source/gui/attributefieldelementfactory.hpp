#ifndef CLOVER_GUI_ATTRIBUTEFIELDFACTORY_HPP
#define CLOVER_GUI_ATTRIBUTEFIELDFACTORY_HPP

#include "build.hpp"
#include "resources/attribute_def.hpp"
#include "element_attributefield.hpp"

namespace clover {
namespace gui {

/// Creates AttributeFieldElements
class AttributeFieldElementFactory {
public:
	/// Dynamically allocated, store in smart pointer
	static BaseAttributeFieldElement* create(const resources::AttributeDef& def);
	
private:
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELDFACTORY_HPP