#ifndef CLOVER_GUI_TEXTLABEL_HPP
#define CLOVER_GUI_TEXTLABEL_HPP

#include "build.hpp"
#include "elementvisualentity_text.hpp"
#include "gui/element.hpp"
#include "visual/model_text.hpp"

namespace clover {
namespace gui {

class TextLabelElement : public Element {
public:
	enum class Alignment {
		Right,
		Center,
		Left
	};
	
	typedef TextElementVisualEntity::Type Type;
	
	TextLabelElement(const util::Str8& text, const util::Coord& pos);
	TextLabelElement(const util::Str8& text, const util::Coord& pos, const util::Coord& row_rad_y);
	TextLabelElement(TextLabelElement&&)= default;
	TextLabelElement& operator=(TextLabelElement&&)= default;
	virtual ~TextLabelElement();
	
	void setText(const util::Str8& text);
	util::Str8 getText() const;
	
	void setAsNumeric(real64 n);
	
	void setType(Type t);
	
	/// Use only for debugging purposes
	/// util::Color of text should be data-driven
	void setColorMul(const util::Color& c);
	
private:
	virtual void spatialUpdate() override;
	void updateRads();
	
	TextElementVisualEntity* textEntity;
	util::Str8 text;
};

} // gui
} // clover

#endif // CLOVER_GUI_TEXTLABEL_HPP