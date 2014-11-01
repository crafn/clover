#include "element_linegraph.hpp"

namespace clover {
namespace gui {

LineGraphElement::LineGraphElement(const util::Coord& offset, const util::Coord& radius):
	PanelElement(offset, radius),
	updateModel(false),
	lineViewXAtCenter(0),
	lineViewRadius(1),
	labelGrid(	util::Coord({0, -radius.getValue().y*0.4}, offset.getType()),
				radius*util::Vec2d{1,0.2}	),
	visualEntity(createCustomVisualEntity()){

	touchable= false;

	lineModel.setMesh(lineMesh);
	modelEntityDef.setModel(lineModel);
	modelEntityDef.setFilled(false);
	//modelEntityDef.setLayer(visual::ModelEntityDef::Layer::Gui);
	modelEntityDef.setEnvLight(1.0);
	visualEntity->getVisualEntity().setDef(modelEntityDef);

	labels.reserve(16);
	
	labelGrid.setDefaultAlignment(util::Vec2d{-1,0});
	labelGrid.setMargin(util::Vec2d{0.1,0});
	addSubElement(labelGrid);
}

LineGraphElement::NodeId LineGraphElement::createLine(const util::Color& c, const util::Str8& name){
	lines.pushBack(Line(c));
	
	// Resize grid
	uint32 y= (labels.size() + 2)/2;
	if (y == 0) y= 1;
	labelGrid.setGridSize(util::Vec2i{3, (int32)y});

	util::Vec2i pos= {	(int32)labels.size()%2,
					(int32)labels.size()/2		};

	labels.pushBack(TextLabelElement(name, util::Coord(util::Vec2d{0,-0.035}*labels.size(), position.getType())));
	labels.back().setColorMul(c);

	labelGrid.addNode(labels.back(), pos);

	return lines.size()-1;
}

LineGraphElement::~LineGraphElement(){
	destroyVisuals();
}

void LineGraphElement::appendPoint(const NodeId& id, const util::Vec2d& p){
	ensure_msg(id < lines.size(), "Invalid id: %i, %i", (int)id, (int)lines.size());
	lines[id].append(p);
	updateModel= true;
}

void LineGraphElement::setViewport(real64 left, real64 right){
	updateModel= true;

	lineViewXAtCenter= (left + right)*0.5;
	lineViewRadius.x= (right - left)*0.5;
}

void LineGraphElement::clearBefore(real64 x){
	for (auto& m: lines){
		m.clearBefore(x);
	}
}

void LineGraphElement::preUpdate(){
	BaseClass::preUpdate();

	if (isActive()){
		modelEntityDef.setColorMul(util::Color{ 1,1,1,1 });
	}

	// Update if active
	if (updateModel && isActive()){
		lineMesh.clear();

		visual::TriMesh mesh; // Curves
		visual::TriMesh bgmesh; // Background-lines
		for (auto& m : lines){
			mesh.add(m.generateMesh(lineViewXAtCenter, lineViewRadius.x));
		}

		// Automatic scaling disabled
		//lineViewRadius.y= mesh.getBoundingBox().y;
		lineViewRadius.y= 1;

		const real32 bg_line_count= 5;
		for (uint32 i=0; i<bg_line_count; ++i){
			// Mark lines behind curves
			util::Vec2f 	top_l= {(real32)(lineViewXAtCenter - lineViewRadius.x), i/(bg_line_count-1)},
					top_r= {(real32)(lineViewXAtCenter + lineViewRadius.x), i/(bg_line_count-1)};
			bgmesh.addTriangle(top_l, top_r, top_r);
			bgmesh.setColor(util::Color{0.5,0.5,0.5,0.5});
		}

		lineMesh.add(bgmesh);
		lineMesh.add(mesh);
		lineMesh.flush();

		updateModel= false;
	}
}

void LineGraphElement::postUpdate(){
	BaseClass::postUpdate();
}

void LineGraphElement::spatialUpdate(){
	BaseClass::spatialUpdate();
	
	util::Coord scale= radius;
	scale *= lineViewRadius.inversed();

	util::Coord pos= position;
	pos -= util::Coord({scale.getValue().x*lineViewXAtCenter, 0}, scale.getType());

	visualEntity->setPositionTarget(pos);
	visualEntity->setRadiusTarget(scale);
}


LineGraphElement::Line::Line(const util::Color& c):
	color(c){
		
}

void LineGraphElement::Line::append(const util::Vec2d& p){
	if (!points.empty())
		ensure(p.x > points.back().x);
		
	points.pushBack(p.casted<util::Vec2f>());
}

void LineGraphElement::Line::clearBefore(real64 x){
	for (uint32 i=0; i<points.size(); ++i){
		if (points[i].x > x){
			points.erase(points.begin(), points.begin()+i);
			break;
		}
	}
}

visual::TriMesh LineGraphElement::Line::generateMesh(real64 center_x, real64 radius_x){
	real64 left= 	center_x - radius_x,
			right= 	center_x + radius_x;

	visual::TriMesh m;
	for(uint32 i=0; i<points.size()-1; ++i){
		if (points[i+1].x < left || points[i].x >= right)
			continue;

		util::Vec2f 	left_p= points[i],
				right_p= points[i+1];

		if (left_p.x < left){
			// Start point is outside
			real32 percentage= (right_p.x - left)/(right_p.x - left_p.x);
			left_p= left_p*percentage + right_p*(1-percentage);
		}

		if (right_p.x > right)
			right_p.x= right;

		m.addTriangle(left_p, right_p, right_p);
	}
	m.setColor(color);
	
	return m;
}

} // gui
} // clover