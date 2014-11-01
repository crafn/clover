#include "element_attributefield.hpp"

namespace clover {
namespace gui {

BaseAttributeFieldElement::BaseAttributeFieldElement()
		: Element(util::Coord::VF(0), util::Coord::VF({0.3, 0.04})){
	elementType= AttributeField;
}

BaseAttributeFieldElement::~BaseAttributeFieldElement(){	
}

} // gui
} // clover