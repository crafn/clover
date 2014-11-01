#ifndef CLOVER_GUI_ELEMENTVISUALENTITY_HPP
#define CLOVER_GUI_ELEMENTVISUALENTITY_HPP

#include "build.hpp"
#include "baseelementvisualentity.hpp"
#include "visual/entity.hpp"

namespace clover {
namespace gui {

class ElementVisualEntity : public BaseElementVisualEntity {
public:
	typedef BaseElementVisualEntity::ContainerType ContainerType;
	enum class Type {
		Panel,
		Custom
	};
	
	ElementVisualEntity(Type t, ContainerType& c);
	ElementVisualEntity(const ElementVisualEntity&)= delete;
	ElementVisualEntity(ElementVisualEntity&&)= default;
	virtual ~ElementVisualEntity();
	
	ElementVisualEntity& operator=(const ElementVisualEntity&)= delete;
	
	void setType(Type t){ type= t; }

	
	virtual void setDepth(int32 d);
	
	virtual void update(){ BaseElementVisualEntity::update(); }
	virtual void setActiveTarget(bool b){ tempSolutionEntity.setActive(b); }
	virtual void setEnabledTarget(bool b);
	virtual void setError(bool b= true);
	virtual void clear() override { tempSolutionEntity.clear(); }
	
	void setPositionTarget(const util::Coord& pos);
	util::Coord getPosition() const;
	
	void setRadiusTarget(const util::Coord& rad);
	util::Coord getRadius() const;
	
	void setHighlightTarget(bool b=true);
	
	visual::Entity& getVisualEntity(){ return tempSolutionEntity; }
	
private:
	Type type;
	visual::Entity tempSolutionEntity;
};

} // gui
} // clover

#endif // CLOVER_GUI_ELEMENTVISUALENTITY_HPP