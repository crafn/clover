#include "entity.hpp"
#include "entity_mgr.hpp"
#include "resources/cache.hpp"
#include "visual_mgr.hpp"

namespace clover {
namespace visual {

Entity::Entity(Layer l)
		: m({true, l, nullptr}){
	if (m.active)
		informActivation(true);
}

Entity::Entity(const EntityDef& def, Layer l)
		: m({true, l, nullptr}){
	setDef(def);
}

Entity::Entity(Entity&& other)
		: m(std::move(other.m)){
	if (m.active){
		other.setActive(false);
		informActivation(true);
	}
}

Entity::~Entity(){
	clear();
}

void Entity::setActive(bool b){
	if (b != m.active)
		informActivation(b);
	
	m.active= b;
}

void Entity::setDef(const EntityDef& def){
	bool was_active= m.active;
	setActive(false);
	
	m.definition= &def;
	resetDefChangeListening();
	
	m.logic= std::unique_ptr<EntityLogic>(m.definition->createLogic());
	m.logic->setLayer(m.layer);
	
	setActive(was_active);
}

void Entity::setDef(const util::Str8& def_name){
	setDef(resources::gCache->getResource<visual::EntityDef>(def_name));
}

void Entity::changeDef(const EntityDef& def){
	if (m.logic){
		bool was_active= m.active;
		setActive(false);
		
		EntityLogic* new_logic= def.createLogic();
		m.definition= &def;
		resetDefChangeListening();
		
		ensure(m.logic.get());
		ensure(new_logic);
		
		new_logic->apply(*m.logic);
		m.logic= std::move(std::unique_ptr<EntityLogic>(new_logic));
		
		setActive(was_active);
	}
	else {
		setDef(def);
	}
}

void Entity::changeDef(const util::Str8& def_name){
	changeDef(resources::gCache->getResource<visual::EntityDef>(def_name));
}

void Entity::apply(const Entity& other){
	ensure(m.logic);
	ensure(m.definition);
	ensure(other.m.logic);
	ensure(m.definition->getType() == other.m.logic->getDef().getType());

	auto prev_t= getTransform();
	m.logic->apply(*other.m.logic);
	onSpatialChange(prev_t);
	m.logic->setLayer(m.layer);
}

void Entity::setLayer(Layer l){ m.layer= l; if (m.logic); m.logic->setLayer(l); }
Entity::Layer Entity::getLayer() const { return m.layer; }

void Entity::setTransform(const Transform& t_){
	ensure(m.logic);
	auto prev_t= getTransform();
	m.logic->setTransform(t_);
	onSpatialChange(prev_t);
}
const Entity::Transform& Entity::getTransform() const { ensure(m.logic); return m.logic->getTransform(); }

void Entity::setDrawPriority(DrawPriority p){ ensure(m.logic); m.logic->setDrawPriority(p); }
Entity::DrawPriority Entity::getDrawPriority() const { ensure(m.logic); return m.logic->getDrawPriority(); }

void Entity::setCoordinateSpace(util::Coord::Type t){
	ensure(m.logic);
	onCoordSpaceChange(t, getScaleCoordinateSpace());
	m.logic->setCoordinateSpace(t);
}

util::Coord::Type Entity::getCoordinateSpace() const { ensure(m.logic); return m.logic->getCoordinateSpace(); }

void Entity::setScaleCoordinateSpace(util::Coord::Type t){
	ensure(m.logic);
	onCoordSpaceChange(getCoordinateSpace(), t);
	m.logic->setScaleCoordinateSpace(t);
}
util::Coord::Type Entity::getScaleCoordinateSpace() const { ensure(m.logic); return m.logic->getScaleCoordinateSpace(); }

void Entity::clear(){
	setActive(false);
	m.logic.reset();
	m.definitionChangeListener.clear();
	m.definition= nullptr;
}

void Entity::informActivation(bool a){
	if (!getLogic() || !getDef())
		return;

	ensure(gVisualMgr);
	gVisualMgr->getEntityMgr().onEntityActivationChange(*this, a);
}

void Entity::onSpatialChange(const Transform& prev_t){
	if (isActive() && prev_t != getTransform()){
		gVisualMgr->getEntityMgr().onEntitySpatialChange(*this, prev_t);
	}	
}

void Entity::onCoordSpaceChange(util::Coord::Type p, util::Coord::Type s){
	if (isActive() && (p != getCoordinateSpace() || s != getScaleCoordinateSpace())){
		gVisualMgr->getEntityMgr().onEntityCoordSpaceChange(*this, p, s);
	}
}

void Entity::resetDefChangeListening(){
	ensure(m.definition);
	m.definitionChangeListener.clear();
	m.definitionChangeListener.listen(*m.definition, [this] (){
		ensure(m.definition);
		bool was_active= m.active;
		setActive(false);
		
		// Recreate logic on change
		EntityLogic* new_logic= m.definition->createLogic();
		if (m.logic)
			new_logic->apply(*m.logic);
		m.logic= std::move(std::unique_ptr<EntityLogic>(new_logic));
		
		setActive(was_active);
	});
}

} // visual
} // clover