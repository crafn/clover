#ifndef CLOVER_GUI_SLIDER_HPP
#define CLOVER_GUI_SLIDER_HPP

#include "build.hpp"
#include "element.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

class SliderElement : public Element {
public:
	DECLARE_GUI_ELEMENT(SliderElement)

	typedef Element BaseType;
	enum Type {
		Horizontal,
		Vertical
	};
	
	SliderElement(Type t, const util::Coord& pos, const util::Coord& length, real32 relative_handle_size= 0.1);
	virtual ~SliderElement();

	void setValue(real32 value);
	void setFixedValue(uint32 value);
	
	real32 getValue() const { return value; }
	uint32 getFixedValue() const { return fixedValue; }

	void setStepCount(uint32 step_count){ stepCount= step_count; }

	virtual void preUpdate();
	virtual void postUpdate();
	virtual void spatialUpdate() override;
	
	bool hasValueChanged() const { return valueChanged; }
	
	GUI_CALLBACK(OnValueChange)
	GUI_CALLBACK(OnValueModify)
	
private:
	void updateHandle();
	
	uint32 getPrimComponent(){ return type; }
	uint32 getSecComponent(){ return 1-type; }
	
	Type type;
	
	ElementVisualEntity* handleVisual;
	ElementVisualEntity* railVisual;

	real32 relHandleSize;
	real32 value;
	uint32 stepCount;
	uint32 fixedValue;
	
	bool valueChanged;
};

} // gui
} // clover

#endif // CLOVER_GUI_SLIDER_HPP