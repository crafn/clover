#include "element_textfieldarray.hpp"

namespace clover {
namespace gui {

TextFieldArrayElement::TextFieldArrayElement(SizeType field_count, const util::Coord& pos, const util::Coord& radx):
	Element(pos, TextFieldElement::defaultRadius(TextFieldElement::Type::Dev).onlyY() + radx.onlyX()){
	
	ensure(field_count);

	for (SizeType i=0; i < field_count; ++i){
		fields.pushBack(std::move(TextFieldElement(TextFieldElement::Type::Dev, util::Coord::VF(0))));
		addSubElement(fields.back());
	}
	
	bindCallbacks();
	updateFields();
}

TextFieldArrayElement::TextFieldArrayElement(TextFieldArrayElement&& other):
	Element(std::move(other)),
	fields(std::move(other.fields)){
	bindCallbacks();
}

TextFieldArrayElement::~TextFieldArrayElement(){
	
}

void TextFieldArrayElement::bindCallbacks(){
	for (auto& m : fields){
	
		m.setOnTextChangeCallback([&] (TextFieldElement& caller) {
			this->OnTextChange(*this);
		});
		m.setOnTextModifyCallback([&] (TextFieldElement& caller) {
			this->OnTextModify(*this);
		});
		
	}
}

TextFieldElement& TextFieldArrayElement::getField(SizeType i){
	ensure(i < fields.size());
	return fields[i];
}

const TextFieldElement& TextFieldArrayElement::getField(SizeType i) const {
	ensure(i < fields.size());
	return fields[i];
}

void TextFieldArrayElement::setRadius(const util::Coord& rad){
	Element::setRadius(rad);
	updateFields();
}

void TextFieldArrayElement::updateFields(){
	util::Coord rad_for_field= getRadius().onlyX() * (1.0 / (real32)fields.size()) + getRadius().onlyY();
	util::Coord pos_for_field= rad_for_field.onlyX()*(1-(int32)fields.size());
	

	for (SizeType i=0; i < fields.size(); ++i){
		fields[i].setRadius(rad_for_field);
		fields[i].setMinRadius(rad_for_field);
		fields[i].setMaxRadius(rad_for_field);
		
		fields[i].setOffset(pos_for_field);
		
		pos_for_field += rad_for_field.onlyX()*2;
	}
}

} // gui
} // clover