#include "baseelementvisualentity.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace gui {

util::DynArray<util::UniquePtr<BaseElementVisualEntity>> BaseElementVisualEntity::visualEntities;

BaseElementVisualEntity::BaseElementVisualEntity(ContainerType& container_):
	container(&container_),
	remove(false),
	onTopOfElement(false){
	visualEntities.pushBack(util::UniquePtr<BaseElementVisualEntity>(this));
	addToContainer();
}

BaseElementVisualEntity::~BaseElementVisualEntity(){
	if (container)
		removeFromContainer();
}

void BaseElementVisualEntity::changeContainer(ContainerType* new_container){
	removeFromContainer();
	container= new_container;
	if (container)
		addToContainer();
	else
		clear(); // "pre-destroy"
}

void BaseElementVisualEntity::update(){
	if (!container)
		remove= true;
}

void BaseElementVisualEntity::commonUpdate(){
	
	auto it= visualEntities.begin();
	while (it != visualEntities.end()){
		BaseElementVisualEntity& entity= *it->get();
		
		if (entity.getContainer() == nullptr){
			entity.update();
		}
		
		if (entity.remove){
			it= visualEntities.erase(it);
		}
		else {
			++it;
		}
	}
}

void BaseElementVisualEntity::addToContainer(){
	ensure(container);
	
	container->pushBack(this);
}

void BaseElementVisualEntity::removeFromContainer(){
	ensure(container);
	
	auto it= container->find(this);
	ensure(it != container->end());
	
	container->erase(it);
}

} // gui
} // clover
