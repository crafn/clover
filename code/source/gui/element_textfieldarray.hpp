#ifndef CLOVER_GUI_ELEMENT_TEXTFIELDARRAY_HPP
#define CLOVER_GUI_ELEMENT_TEXTFIELDARRAY_HPP

#include "build.hpp"
#include "element_textfield.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace gui {

class TextFieldArrayElement : public Element {
public:
	DECLARE_GUI_ELEMENT(TextFieldArrayElement)

	TextFieldArrayElement(SizeType field_count, const util::Coord& pos, const util::Coord& radx);
	TextFieldArrayElement(TextFieldArrayElement&&);
	virtual ~TextFieldArrayElement();

	const TextFieldElement& operator[](SizeType i) const { return getField(i); }
	TextFieldElement& operator[](SizeType i){ return getField(i); }

	const TextFieldElement& getField(SizeType i) const;
	TextFieldElement& getField(SizeType i);

	SizeType size() const { return fields.size(); }

	virtual void setRadius(const util::Coord& rad);

	typedef util::DynArray<TextFieldElement>::Iter Iter;

	Iter begin(){ return fields.begin(); }
	Iter end(){ return fields.end(); }

	GUI_CALLBACK(OnTextChange) // Assigned to trigger when fields' OnTextModify is triggered
	GUI_CALLBACK(OnTextModify)

private:
	void bindCallbacks();
	void updateFields();

	util::DynArray<TextFieldElement> fields;
};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENT_TEXTFIELDARRAY_HPP