#include "element_textlabel.hpp"

#include <sstream>

namespace clover {
namespace gui {

TextLabelElement::TextLabelElement(const util::Str8& t, const util::Coord& pos):
		Element(pos, util::Coord(0,util::Coord::View_Pixels)),
		textEntity(createTextVisualEntity(TextElementVisualEntity::Type::Dev)){
	elementType= TextLabel;
			
	touchable= false;
	textEntity->setText(t);
	text= t;

	updateRads();
}

TextLabelElement::TextLabelElement(const util::Str8& t, const util::Coord& pos, const util::Coord& row_rad_y):
		Element(pos, util::Coord(0,util::Coord::View_Pixels)),
		textEntity(createTextVisualEntity(TextElementVisualEntity::Type::Dev)){
	elementType= TextLabel;
	touchable= false;
	
	textEntity->setFontSizeByVerticalRadius(row_rad_y);
	textEntity->setText(t);
	text= t;
	updateRads();
}

TextLabelElement::~TextLabelElement(){
	
}

void TextLabelElement::setText(const util::Str8& t){
	text= t;
	textEntity->setText(text);
	updateRads();
}

util::Str8 TextLabelElement::getText() const {
	return text;
}

void TextLabelElement::setAsNumeric(real64 n){
	std::string str;
	std::stringstream ss;
	ss << n;
	ss >> str;
	setText(util::Str8(str));
}

void TextLabelElement::setType(Type t){
	textEntity->setType(t);
}

void TextLabelElement::setColorMul(const util::Color& c){
	textEntity->setColorMul(c);
}

void TextLabelElement::spatialUpdate(){
	Element::spatialUpdate();
	textEntity->setPositionTarget(util::Coord::P({0, 2}) + getPosition());
}

void TextLabelElement::updateRads(){
	util::Coord rad= textEntity->getDimensions()*0.5;
	
	rad.y= 8; /// @todo Font dependent
	setRadius(rad);
	setMinRadius(rad);
	setMaxRadius(rad);
}

} // gui
} // clover
