#ifndef CLOVER_GUI_BUTTON_HPP
#define CLOVER_GUI_BUTTON_HPP

#include "build.hpp"
#include "element.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/model_text.hpp"

namespace clover {
namespace gui {

class ButtonElement : public Element {
public:
	ButtonElement(const util::Coord& pos= util::Coord::P(0), const util::Coord& rad= util::Coord::P(0));
	ButtonElement(const util::Str8& text, const util::Coord& pos= util::Coord::P(0), const util::Coord& rad= util::Coord::P(0));
	ButtonElement(ButtonElement&&)= default;
	virtual ~ButtonElement();
	
	ButtonElement& operator=(ButtonElement&&)= default;
	
	void setText(const util::Str8& text);
	const util::Str8& getText() const;

	virtual void postUpdate();

protected:
	virtual void spatialUpdate() override;
	void init();
	
	util::Str8 text;
	ElementVisualEntity* buttonVisual;
	TextElementVisualEntity* textVisual;
	real32 drawRadiusMul;
};

} // gui
} // clover

#endif // CLOVER_GUI_BUTTON_HPP
