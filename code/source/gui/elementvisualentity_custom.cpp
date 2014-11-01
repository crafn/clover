#include "elementvisualentity_custom.hpp"

namespace clover {
namespace gui {

CustomElementVisualEntity::~CustomElementVisualEntity(){
}

void CustomElementVisualEntity::setPositionTarget(const util::Coord& pos){
	entity.setCoordinateSpace(pos.getType());
	entity.setPosition(pos.getValue().xyz());
}

util::Coord CustomElementVisualEntity::getPosition() const {
	return util::Coord(entity.getPosition().xy(), entity.getCoordinateSpace());
}

void CustomElementVisualEntity::setRadiusTarget(const util::Coord& rad){
	entity.setScaleCoordinateSpace(rad.getType());
	entity.setScale(rad.getValue().xyz());
}

util::Coord CustomElementVisualEntity::getRadius() const {
	return util::Coord(entity.getScale().xy(), entity.getScaleCoordinateSpace(), true);;
}

} // gui
} // clover