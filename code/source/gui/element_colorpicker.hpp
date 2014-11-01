#ifndef CLOVER_GUI_COLORPICKER_HPP
#define CLOVER_GUI_COLORPICKER_HPP

#include "build.hpp"
#include "element.hpp"
#include "visual/mesh.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

class ColorPickerElement : public Element {
public:
	DECLARE_GUI_ELEMENT(ColorPickerElement);
	
	typedef Element BaseType;
	
	ColorPickerElement(const util::Coord& offset, const util::Coord& rad);
	ColorPickerElement(ColorPickerElement&&)= delete;
	virtual ~ColorPickerElement();
	
	virtual void postUpdate();
	
	void setColor(const util::Color&);
	util::Color getColor() const;

	/// @todo Resizing stuff

	GUI_CALLBACK(OnColorModify)
private:
	
	static constexpr real32 wheelWidth= 0.13;
	static constexpr real32 spatialVerticalRadius= (1-wheelWidth)*0.6;
	static constexpr real32 spatialVerticalOffset= 0.2;
	
	static real32 getSpatialHorizontalRadius(){
		// a= sqrt(c^2 - b^2)
		return sqrt(pow(1-wheelWidth, 2) - pow(spatialVerticalRadius + spatialVerticalOffset, 2));
	}

	void updateVisuals();
	void updateSpatialMesh();
	
	virtual void spatialUpdate() override;

	real32 wheel;
	real32 alpha;
	util::Vec2f spatial;
	
	bool draggingWheel;
	bool draggingSpatial;
	
	visual::TriMesh wheelMesh;
	visual::Model wheelModel;
	visual::ModelEntityDef wheelVisualEntityDef;
	CustomElementVisualEntity* wheelElementEntity;
	
	// Saturation and lightness
	visual::TriMesh spatialMesh;
	visual::Model spatialModel;
	visual::ModelEntityDef spatialVisualEntityDef;
	CustomElementVisualEntity* spatialElementEntity;
};

} // gui
} // clover

#endif // CLOVER_GUI_COLORPICKER_HPP