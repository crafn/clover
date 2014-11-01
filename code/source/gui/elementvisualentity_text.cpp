#include "elementvisualentity_text.hpp"
#include "visual/font.hpp"

namespace clover {
namespace gui {

TextElementVisualEntity::TextElementVisualEntity(Type t, ContainerType& c):
	BaseElementVisualEntity(c),
	entity(visual::Entity::Layer::Gui){
	
	model.setActiveFont("Dev", visual::Font::Size::Normal);
	model.setAlignment({0.5,0.5}); // Centered
	entityDef.setModel(model);
	entityDef.setSnappingToPixels();
	entityDef.setEnvLight(1.0);
	
	entity.setDef(entityDef);
	entity.setScale(util::Vec3d(1));
	entity.setCoordinateSpace(util::Coord::View_Pixels);
	entity.setScaleCoordinateSpace(util::Coord::View_Pixels);
}

TextElementVisualEntity::~TextElementVisualEntity(){
}

void TextElementVisualEntity::setType(Type t){
	type= t;
	if (type == Type::Dev){
		model.setActiveFont("Dev");
	}
	else {
		model.setActiveFont("Dev_Light");
		/// @todo Replace this hack when font resources are ready
		entityDef.setColorMul(util::Color{0.5, 0.5, 0.5, 1.0});
	}
}

void TextElementVisualEntity::setFontSizeByVerticalRadius(const util::Coord& rad){
	/// @todo Proper sizing
	if (rad.converted(util::Coord::View_Pixels).y < 10)
		model.setActiveFontSize(visual::Font::Size::Small);
	else
		model.setActiveFontSize(visual::Font::Size::Normal);
}

void TextElementVisualEntity::setText(const util::Str8& text){
	model.setText(text);
}

void TextElementVisualEntity::setColorMul(const util::Color& color){
	entityDef.setColorMul(color);
	colorMul= color;
}

void TextElementVisualEntity::setDepth(int32 d){
	entity.setDrawPriority(d);
}

void TextElementVisualEntity::update(){
	BaseElementVisualEntity::update();
}

void TextElementVisualEntity::setActiveTarget(bool b){
	entity.setActive(b);
}

void TextElementVisualEntity::setEnabledTarget(bool b){
	if (b)
		entityDef.setColorMul(util::Color{1.0, 1.0, 1.0, 1.0});
	else
		entityDef.setColorMul(util::Color{0.4, 0.4, 0.4, 0.8});
}

void TextElementVisualEntity::setError(bool b){
	if (b)
		entityDef.setColorMul(util::Color{1.0, 0.0, 0.0, 1.0});
	else
		entityDef.setColorMul(colorMul);
}

void TextElementVisualEntity::setPositionTarget(const util::Coord& pos){
	entity.setPosition(pos.converted(util::Coord::View_Pixels).getValue().xyz());
	entity.setCoordinateSpace(util::Coord::View_Pixels);
}

} // gui
} // clover