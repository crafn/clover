#include "element_layout_radial.hpp"

namespace clover {
namespace gui {

RadialLayoutElement::RadialLayoutElement(const util::Coord& pos, const util::Coord& rad):
		LayoutElement(pos, rad){
}

RadialLayoutElement::~RadialLayoutElement(){
}

void RadialLayoutElement::preUpdate(){
	spatialUpdate();
	LayoutElement::preUpdate();
}

void RadialLayoutElement::updateNodes(){
	if (nodes.empty()) return;
	
	nodes.sort();
	
	real64 angle_addition= util::tau/nodes.size();
	real64 angle= -util::pi*0.5 + angle_addition*0.5;
	
	for (auto &m : nodes){
		Element& e= m.getElement();
		util::Coord target_offset= e.getOffset();
		target_offset= util::Coord(util::Vec2d{cos(angle)*radius.x, sin(angle)*radius.y}, radius.getType());
		e.setOffset(target_offset);
		
		angle += angle_addition;
	}
}

void RadialLayoutElement::spatialUpdate() {
	BaseType::spatialUpdate();
	updateNodes();
}

} // gui
} // clover