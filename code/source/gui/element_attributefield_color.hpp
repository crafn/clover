#ifndef CLOVER_GUI_ATTRIBUTEFIELD_COLOR_HPP
#define CLOVER_GUI_ATTRIBUTEFIELD_COLOR_HPP

#include "build.hpp"
#include "element_attributefield.hpp"
#include "element_colorpicker.hpp"
#include "element_floating.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

template <>
class AttributeFieldElement<resources::ColorAttributeType> : public AttributeFieldWrapElement<resources::ColorAttributeType> {
public:
	typedef AttributeFieldWrapElement<resources::ColorAttributeType> BaseType;

	AttributeFieldElement(const resources::AttributeDef& def);
	virtual ~AttributeFieldElement();
	
	virtual void setActive(bool b=true);
	virtual void preUpdate();
	virtual void postUpdate();
	
private:
	virtual void onValueSet();
	virtual void spatialUpdate() override;
	
	visual::ModelEntityDef colorPreviewEntityDef;
	CustomElementVisualEntity* previewElementEntity;
	
	util::Coord colorPickerRadius() const { return util::Coord::VF(0.4); }
	
	gui::FloatingElement colorPickerFloating;
	gui::ColorPickerElement colorPicker;
	
};

} // gui
} // clover

#endif // CLOVER_GUI_ATTRIBUTEFIELD_COLOR_HPP