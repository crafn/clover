#ifndef CLOVER_GUI_ELEMENTVISUALENTITY_TEXT_HPP
#define CLOVER_GUI_ELEMENTVISUALENTITY_TEXT_HPP

#include "build.hpp"
#include "baseelementvisualentity.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/model_text.hpp"

namespace clover {
namespace gui {

class TextElementVisualEntity : public BaseElementVisualEntity {
public:
	typedef BaseElementVisualEntity::ContainerType ContainerType;
	
	enum class Type {
		Dev,
		Dev_Light//,
		// Header,
		// Content,
		// ...
		
	};
	
	TextElementVisualEntity(Type t, ContainerType& c);
	TextElementVisualEntity(TextElementVisualEntity&&)= default;
	
	virtual ~TextElementVisualEntity();
	
	void setType(Type t);
	
	void setFontSizeByVerticalRadius(const util::Coord& rad);
	
	void setText(const util::Str8& text);
	
	/// Use only for debugging purposes
	/// util::Color of text should be data-driven
	void setColorMul(const util::Color& color);
	
	util::Coord getDimensions() const { return model.getDimensions(); }
	util::Coord getRadius() const { return getDimensions()*0.5; }
	
	virtual void setDepth(int32 d);
	
	virtual void update();
	virtual void setActiveTarget(bool b);
	virtual void setEnabledTarget(bool b);
	virtual void setError(bool b= true);
	virtual void clear() override { entity.clear(); }
	
	void setPositionTarget(const util::Coord& pos);
	
private:
	Type type;
	
	visual::TextModel model;
	visual::ModelEntityDef entityDef;
	visual::Entity entity;
	
	util::Color colorMul;

};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENTVISUALENTITY_TEXT_HPP