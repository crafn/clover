#ifndef CLOVER_GUI_BASEELEMENTVISUALENTITY_HPP
#define CLOVER_GUI_BASEELEMENTVISUALENTITY_HPP

#include "build.hpp"
#include "util/coord.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace gui {

/// Visual representation for Elements to use
/// Must be dynamically allocated because commonUpdate() deletes old entities
class BaseElementVisualEntity {
public:
	typedef util::DynArray<BaseElementVisualEntity*> ContainerType;

	BaseElementVisualEntity(ContainerType& container);
	BaseElementVisualEntity(const BaseElementVisualEntity&)= delete;
	BaseElementVisualEntity(BaseElementVisualEntity&&)= delete;
	virtual ~BaseElementVisualEntity();

	void changeContainer(ContainerType* new_container);
	const ContainerType* getContainer() const { return container; }

	virtual void setDepth(int32 i){}

	/// util::Set on top of subElements, default is false
	void setOnTopOfElement(bool b=true){ onTopOfElement= b; }
	bool isOnTopOfElement() const { return onTopOfElement; }

	virtual void update();
	virtual void setActiveTarget(bool b) = 0;
	virtual void setEnabledTarget(bool b) = 0;

	/// Used to indicate error in something that gui element represents
	/// Usually turns element red or something similar
	virtual void setError(bool b= true) = 0;

	virtual void clear(){}

	/// Removes old entities
	static void commonUpdate();
	static void clearAll(){ visualEntities.clear(); }
	static SizeType getEntityCount(){ return visualEntities.size(); }

private:
	ContainerType* container;
	bool remove;

	bool onTopOfElement;

	void addToContainer();
	void removeFromContainer();

	static util::DynArray<std::unique_ptr<BaseElementVisualEntity>> visualEntities;
};

} // gui
} // clover

#endif // CLOVER_GUI_BASEELEMENTVISUALENTITY_HPP