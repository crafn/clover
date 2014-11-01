#ifndef CLOVER_GUI_ELEMENTVISUALENTITY_CUSTOM_HPP
#define CLOVER_GUI_ELEMENTVISUALENTITY_CUSTOM_HPP

#include "build.hpp"
#include "baseelementvisualentity.hpp"
#include "visual/entity.hpp"

namespace clover {
namespace gui {

/// @todo Remove, replace with ElementVisualEntity::Type::Custom
/// Move-constructor is problematic with Elements, because this takes pointer to the entity
class CustomElementVisualEntity : public BaseElementVisualEntity {
public:
	typedef BaseElementVisualEntity::ContainerType ContainerType;

	
	CustomElementVisualEntity(ContainerType& c):
		BaseElementVisualEntity(c), entity(visual::Entity::Layer::Gui){
	}
	
	CustomElementVisualEntity(CustomElementVisualEntity&&)= default;
	
	virtual ~CustomElementVisualEntity();
	
	visual::Entity& getVisualEntity(){ return entity; }
	
	virtual void setDepth(int32 d){ entity.setDrawPriority(d); }
	
	virtual void update(){ BaseElementVisualEntity::update(); }
	virtual void setActiveTarget(bool b){ entity.setActive(b); }
	virtual void setEnabledTarget(bool b){}; /// Wat do
	virtual void setError(bool b= true){};
	virtual void clear() override { entity.clear(); }
	
	void setPositionTarget(const util::Coord& pos);
	util::Coord getPosition() const;
	
	void setRadiusTarget(const util::Coord& rad);
	util::Coord getRadius() const;
	
private:

	visual::Entity entity;

};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENTVISUALENTITY_CUSTOM_HPP