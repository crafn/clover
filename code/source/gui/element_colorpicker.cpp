#include "element_colorpicker.hpp"
#include "ui/userinput.hpp"
#include "hardware/mouse.hpp"

namespace clover {
namespace gui {

ColorPickerElement::ColorPickerElement(const util::Coord& offset, const util::Coord& rad)
		: Element(offset, rad)
		, wheel(0)
		, alpha(1)
		, spatial({1,1})
		, draggingWheel(false)
		, draggingSpatial(false)
		, wheelElementEntity(createCustomVisualEntity())
		, spatialElementEntity(createCustomVisualEntity()){
	elementType= Element::ColorPicker;
	
	const uint32 stepCount= 50;
	for (uint32 step=0; step<stepCount; ++step){
		real32 phase= (real32)step/stepCount;
		real32 next_phase= (real32)(step+1)/stepCount;
		
		real32 phase_rad= phase*util::tau;
		real32 next_phase_rad= next_phase*util::tau;
		
		util::Vec2f a= {(real32)cos(phase_rad),			(real32)sin(phase_rad)};
		util::Vec2f b= {(real32)cos(next_phase_rad),	(real32)sin(next_phase_rad)};
		
		util::Color color;
		color.setHsl({phase, 1, 0.5});
		
		util::Color next_color;
		next_color.setHsl({next_phase, 1, 0.5});
		
		real32 widthmul= 1-wheelWidth;
		wheelMesh.addTriangle(	a, b, a*widthmul,
								color, next_color, color);

		wheelMesh.addTriangle(	a*widthmul, b, b*widthmul,
								color, next_color, next_color);
	
	}

	wheelMesh.flush();
	wheelModel.setMesh(wheelMesh);
	
	wheelVisualEntityDef.setModel(wheelModel);
//	wheelVisualEntityDef.setLayer(visual::ModelEntityDef::Layer::Gui_Front);
	wheelVisualEntityDef.setEnvLight(1.0);
	wheelElementEntity->getVisualEntity().setDef(wheelVisualEntityDef);
	wheelElementEntity->getVisualEntity().setScale(radius.getValue().xyz());
	wheelElementEntity->getVisualEntity().setScaleCoordinateSpace(radius.getType());
	
	updateSpatialMesh();
	
	spatialModel.setMesh(spatialMesh);
	
	spatialVisualEntityDef.setModel(spatialModel);
//	spatialVisualEntityDef.setLayer(visual::ModelEntityDef::Layer::Gui_Front);
	spatialVisualEntityDef.setEnvLight(1.0);
	spatialElementEntity->getVisualEntity().setDef(spatialVisualEntityDef);
	spatialElementEntity->getVisualEntity().setScale(radius.getValue().xyz());
	spatialElementEntity->getVisualEntity().setScaleCoordinateSpace(radius.getType());
}

ColorPickerElement::~ColorPickerElement(){
	destroyVisuals();
}

void ColorPickerElement::postUpdate(){
	Element::postUpdate();
	
	if (!isActive()) return;
	
	util::Coord cursor_offset= (guiCursor->getPosition() - getPosition());
	
	real32 cursor_dist_from_center= (cursor_offset.converted(radius.getType()).getValue()
									*radius.getValue().inversed()).length();
	
	if (draggingWheel){
		
		
		if (hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_RIGHT)){
			alpha= (cursor_dist_from_center-(1-wheelWidth))*1.5 + 1 - wheelWidth;
			alpha= util::limited(alpha, 0.0f, 1.0f);
		}
		else if (hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_LEFT)){
			real32 phase= cursor_offset.converted(util::Coord::View_Fit).getValue().rotationZ()/util::tau;
			wheel= phase;
		}
		
		//print(debug::Ch::Gui, debug::Vb::Trivial, "Wheel: %f, alpha: %f", wheel, alpha);
		
		updateSpatialMesh();
		
		OnColorModify(*this);
	}
	else if (draggingSpatial){
		
		// Origo in lower left corner, upper right (1,1)
		util::Vec2d point_in_box= cursor_offset.converted(radius.getType()).getValue()*radius.getValue().inversed();
		
		point_in_box.y += spatialVerticalRadius - spatialVerticalOffset;
		point_in_box.x += getSpatialHorizontalRadius();
		
		point_in_box.x /= getSpatialHorizontalRadius()*2;
		point_in_box.y /= spatialVerticalRadius*2;
		
		point_in_box.x= util::limited(point_in_box.x, 0.0, 1.0);
		point_in_box.y= util::limited(point_in_box.y, 0.0, 1.0);
		
		spatial= point_in_box.casted<util::Vec2f>();
		
		//print(debug::Ch::Gui, debug::Vb::Trivial, "Spatial: %f, %f", spatial.x, spatial.y);
		
		OnColorModify(*this);
	}
	
	if (!draggingWheel && !draggingSpatial && 
		(guiCursor->getDraggedElement() == this || gUserInput->isTriggered(UserInput::GuiCause) ||
			hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_RIGHT))){
			
		// Pressed or dragged this element
			
		if (cursor_dist_from_center < 1 - wheelWidth && !hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_RIGHT)){
			// Inside circle
			draggingSpatial= true;
		}
		else {
			// On or outside circle
			draggingWheel= true;
		}
	}
	
	if (guiCursor->getDraggedElement() != this && !hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_RIGHT)){
		draggingWheel= false;
		draggingSpatial= false;
	}
}

void ColorPickerElement::setColor(const util::Color& c){
	if (c == getColor())
		return;
	
	auto hsv= c.getHsv();
	wheel= hsv[0];
	spatial= util::Vec2f{hsv[1], hsv[2]};
	alpha= c.a;
	
	updateSpatialMesh();
}

util::Color ColorPickerElement::getColor() const {
	util::Color c;
	c.setHsv({wheel, spatial[0], spatial[1]});
	c.a= alpha;
	
	return c;
}

void ColorPickerElement::updateSpatialMesh(){
	const uint32 rowCount= 10;
	const uint32 columnCount= 10;
	
	real32 horizontal_rad= getSpatialHorizontalRadius();
	
	util::Vec2f coord_step_xy= {horizontal_rad/columnCount*2, spatialVerticalRadius/rowCount*2};
	util::Vec2f coord_step_x= {coord_step_xy.x, 0};
	util::Vec2f coord_step_y= {0, coord_step_xy.y};
	
	util::Vec2f spatial_step_xy= {1.0/columnCount, 1.0/rowCount};
	
	spatialMesh.clear();
	for (uint32 step_y=0; step_y<rowCount; ++step_y){
		for (uint32 step_x=0; step_x<columnCount; ++step_x){
			
			util::Vec2f spatial_pos= { (real32)step_x/columnCount, (real32)step_y/rowCount };
			
			util::Vec2f coord_pos= {
				-horizontal_rad + horizontal_rad*spatial_pos.x*2,
				spatialVerticalOffset - spatialVerticalRadius + spatialVerticalRadius*spatial_pos.y*2
			};
			
			util::Color color, color_step_xy, color_step_x, color_step_y;
			color.setHsv({wheel, spatial_pos.x, spatial_pos.y});
			color_step_xy.setHsv({wheel, spatial_pos.x + spatial_step_xy.x, spatial_pos.y + spatial_step_xy.y});
			color_step_x.setHsv({wheel, spatial_pos.x + spatial_step_xy.x, spatial_pos.y});
			color_step_y.setHsv({wheel, spatial_pos.x, spatial_pos.y + spatial_step_xy.y});
			
			spatialMesh.addTriangle(coord_pos, coord_pos + coord_step_x, coord_pos + coord_step_xy,
									color, color_step_x, color_step_xy);
			
			spatialMesh.addTriangle(coord_pos, coord_pos + coord_step_xy, coord_pos + coord_step_y,
									color, color_step_xy, color_step_y);
		}
	}
	spatialMesh.flush();
}

void ColorPickerElement::spatialUpdate(){
	BaseType::spatialUpdate();
	
	if (isActive()){
		wheelElementEntity->setPositionTarget(getPosition());
		spatialElementEntity->setPositionTarget(getPosition());
	}
}

} // gui
} // clover