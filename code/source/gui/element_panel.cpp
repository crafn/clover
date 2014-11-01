#include "element_panel.hpp"

namespace clover {
namespace gui {

PanelElement::PanelElement(const util::Coord& offset, const util::Coord& rad):
				Element(offset, rad),
				visual(createVisualEntity(ElementVisualEntity::Type::Panel)){
	elementType= Panel;
}

PanelElement::~PanelElement(){
}

void PanelElement::spatialUpdate(){
	Element::spatialUpdate();
	
	visual->setPositionTarget(position);
	visual->setRadiusTarget(radius);
}

} // gui
} // clover