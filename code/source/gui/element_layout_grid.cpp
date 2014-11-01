#include "element_layout_grid.hpp"

namespace clover {
namespace gui {

GridLayoutElement::GridLayoutElement(const util::Coord& offset, const util::Coord& rad, util::Vec2i grid_size):
		LayoutElement(offset, rad),
		gridSize(grid_size){
	elementType= GridLayout;
}

GridLayoutElement::~GridLayoutElement(){
}

void GridLayoutElement::setMargin(const util::Vec2d& m){
	relativeMargin= m;
}

void GridLayoutElement::setGridSize(const util::Vec2i& size){
	gridSize= size;
}

void GridLayoutElement::setDefaultAlignment(const util::Vec2d& alignment){
	defaultAlignment= alignment;
}

bool GridLayoutElement::isEmpty(const util::Vec2i& pos){
	ensure(pos.x >= 0 && pos.y >= 0 && pos.x < gridSize.x && pos.y < gridSize.y);

	for (auto& m : nodes){
		if (m.getPosition() == pos) return false;
	}

	return true;
}

void GridLayoutElement::addNode(Element& e, Node::PositionType node_pos){
	BaseType::addNode(e, node_pos);
	nodes.back().setAlignment(defaultAlignment);
	ensure(node_pos.x >= 0 && node_pos.y >= 0 && node_pos.x < gridSize.x && node_pos.y < gridSize.y);
}

void GridLayoutElement::preUpdate(){
	LayoutElement::preUpdate();
}

void GridLayoutElement::updateNodes(){
	// 		2*radius = (gridSize - util::Vec2i(1))*spacing + 2*relativeMargin*spacing
	// ->	spacing = 2*radius/(gridSize - util::Vec2i(1) + 2*relativeMargin)

	util::Vec2d scaling= util::Vec2d(gridSize.casted<util::Vec2d>() - util::Vec2d(1) + relativeMargin*2);
	util::Coord spacing= util::Coord(	radius.getValue()*scaling.inversed()*2, 
							radius.getType());
	if (radius.getValue().hasZeroComponent() || scaling.hasZeroComponent()){
		spacing.setValue(util::Vec2d(0.0)); // No inifinities please
	}
	util::Coord margin= spacing*relativeMargin;
	
	for (auto &m : nodes){
		
		Element& e= m.getElement();
		
		util::Vec2i grid_pos= m.getPosition()*util::Vec2i{1,-1} + util::Vec2i{0, gridSize.y-1};
		
		util::Coord pos( (margin + spacing*grid_pos.casted<util::Vec2d>() - radius).getValue(), spacing.getType());
		e.setOffset(pos - e.getRadius()*m.getAlignment());
	}
}

void GridLayoutElement::spatialUpdate(){
	BaseType::spatialUpdate();
	updateNodes();
}

} // gui
} // clover