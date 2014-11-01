#include "elementvisualentity.hpp"

namespace clover {
namespace gui {

ElementVisualEntity::ElementVisualEntity(Type t, ContainerType& c):
	BaseElementVisualEntity(c),
	type(t),
	tempSolutionEntity(visual::Entity::Layer::Gui){
		
	tempSolutionEntity.setDef("guiPanel");
	tempSolutionEntity.setActive(true);
	tempSolutionEntity.setPosition(util::Vec3d(0));
	tempSolutionEntity.setCoordinateSpace(util::Coord::View_Fit);
	tempSolutionEntity.setScaleCoordinateSpace(util::Coord::View_Fit);
}

ElementVisualEntity::~ElementVisualEntity(){
}

void ElementVisualEntity::setDepth(int32 d){
	tempSolutionEntity.setDrawPriority(d);
}

void ElementVisualEntity::setEnabledTarget(bool b){
	if (b)
		tempSolutionEntity.changeDef("guiPanel");
	else
		tempSolutionEntity.changeDef("guiPanel_disabled");
}

void ElementVisualEntity::setError(bool b){
	if (b)
		tempSolutionEntity.changeDef("guiPanel_error");
	else
		tempSolutionEntity.changeDef("guiPanel");
}

void ElementVisualEntity::setPositionTarget(const util::Coord& pos){
	tempSolutionEntity.setCoordinateSpace(util::Coord::View_Fit);
	tempSolutionEntity.setPosition(pos.converted(util::Coord::View_Fit).getValue().xyz());
}

util::Coord ElementVisualEntity::getPosition() const {
	return util::Coord(tempSolutionEntity.getPosition().xy(), tempSolutionEntity.getCoordinateSpace());
}

void ElementVisualEntity::setRadiusTarget(const util::Coord& rad){
	tempSolutionEntity.setScaleCoordinateSpace(util::Coord::View_Fit);
	tempSolutionEntity.setScale(rad.converted(util::Coord::View_Fit).getValue().xyz());
}

util::Coord ElementVisualEntity::getRadius() const {
	util::Coord c(tempSolutionEntity.getScale().xy(), tempSolutionEntity.getScaleCoordinateSpace());
	c.setRelative();
	return c;
}

void ElementVisualEntity::setHighlightTarget(bool b){
	if (b)
		tempSolutionEntity.changeDef("guiPanel_highlight");
	else
		tempSolutionEntity.changeDef("guiPanel");
}

} // gui
} // clover