#ifndef CLOVER_GUI_CHECKBOX_HPP
#define CLOVER_GUI_CHECKBOX_HPP

#include "build.hpp"
#include "element.hpp"
#include "element_button.hpp"
#include "element_textlabel.hpp"
#include "visual/entity_def_model.hpp"

namespace clover {
namespace gui {

class CheckBoxElement : public Element {
public:
	DECLARE_GUI_ELEMENT(CheckBoxElement)
	
	CheckBoxElement(const util::Str8& label, const util::Coord& pos= util::Coord::VF(0), const util::Coord& box_rad=util::Coord::VF(0.03));
	CheckBoxElement(CheckBoxElement&&)= default;
	CheckBoxElement& operator=(CheckBoxElement&&)= default;
	virtual ~CheckBoxElement();
	
	virtual void preUpdate();
	virtual void postUpdate();

	void setChecked(bool b=true);
	
	bool isChecked() const { return checked; }
	
	void setValue(bool b){ setChecked(b); }
	bool getValue() const { return isChecked(); }

	bool isToggled() const;
	
	virtual void spatialUpdate() override;
	
	GUI_CALLBACK(OnValueModify)
	
protected:
	void updateOffsets();
	
	bool checked;
	util::Coord boxRadius;
	gui::ButtonElement btn; // Klikkauksen tunnistamista varten
	TextLabelElement label; // Teksti
	ElementVisualEntity* checkVisual;
};

} // gui
} // clover

#endif // CLOVER_GUI_CHECKBOX_HPP