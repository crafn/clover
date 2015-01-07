#include "model_text.hpp"
#include "debug/print.hpp"
#include "global/env.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace visual {

TextModel::TextModel():
		activeFont(0),
		activeSize(Font::Size::Normal),
		relativeOrigo({0,1}){
	Model::mesh= &textMesh;
}

TextModel::~TextModel(){

}

TextModel& TextModel::operator=(const TextModel& re){
	Model::operator=(re);

	activeFont= re.activeFont;
	activeSize= re.activeSize;

	textMesh= re.textMesh;
	textMesh.flush();
	mesh= &textMesh;

	relativeOrigo= re.relativeOrigo;
	dimensions= re.dimensions;

	return *this;
}

TextModel::TextModel(const TextModel& re){
	this->operator=(re);
}

TextModel::TextModel(TextModel&& other):
		Model(std::move(other)),
		activeFont(other.activeFont),
		activeSize(other.activeSize),
		textMesh(std::move(other.textMesh)),
		relativeOrigo(other.relativeOrigo),
		dimensions(other.dimensions){
	mesh= &textMesh;
}

void TextModel::setActiveFont(const util::Str8& name){
	setActiveFont(name, activeSize);
}

void TextModel::setActiveFont(const util::Str8& fontname, Font::Size size){
	activeFont= &global::g_env.resCache->getFont(fontname);
	activeSize= size;

	setMaterial(activeFont->getMaterial());
}

void TextModel::setActiveFontSize(Font::Size size){
	activeSize= size;
}

util::Vec2d TextModel::getTranslation(){
	return util::Vec2d(util::Vec2d{-relativeOrigo.x, -relativeOrigo.y+1}*dimensions + util::Vec2d(0.5)).floored();
}

void TextModel::setAlignment(util::Vec2d relative_origo){
	textMesh.translate(-getTranslation().casted<util::Vec3f>());
	relativeOrigo= relative_origo;
	textMesh.translate(getTranslation().casted<util::Vec3f>());
	textMesh.flush();
}

util::Vec2d TextModel::getAlignment(){
	return relativeOrigo;
}

util::Coord TextModel::getDimensions() const {
	return util::Coord(dimensions, util::Coord::View_Pixels);
}

void TextModel::setText(const char8 *str, ...){
	va_list argList;
	va_start(argList, str);
	util::Str8 text;

	text.setFormattedArgList(str, argList);
	va_end(argList);

	setText(text);
}

void TextModel::setText(const util::Str8& text){
	if (!activeFont)
		throw global::Exception("RETextModel::setText(..): no active font");

	textMesh.clear();

	TriMesh am;
	am= activeFont->generateMesh(activeSize, text);

	textMesh.add(am);
	
	if (textMesh.getBoundingBox().isSet())
		dimensions= textMesh.getBoundingBox().getSize().casted<util::Vec2d>();
	else
		dimensions= util::Vec2d(0);


	textMesh.translate(getTranslation().casted<util::Vec3f>());
	textMesh.flush();
}

} // visual
} // clover
