#ifndef CLOVER_VISUAL_ENTITY_HPP
#define CLOVER_VISUAL_ENTITY_HPP

#include "build.hpp"
#include "entitylogic.hpp"
#include "entity_def.hpp"
#include "util/cb_listener.hpp"

namespace clover {
namespace visual {

class Entity {
public:
	typedef EntityLogic::Transform Transform;
	typedef EntityDef::DrawPriority DrawPriority;
	
	using Layer = EntityLogic::Layer;
	
	Entity(Layer l= Layer::World);
	Entity(const EntityDef& def, Layer l= Layer::World);
	Entity(const Entity&)= delete;
	Entity(Entity&&);
	virtual ~Entity();
	
	Entity& operator=(const Entity&)= delete;
	Entity& operator=(Entity&&)= delete;
	
	void setActive(bool b= true);
	bool isActive() const { return m.active; }
	
	/// Sets definition and (re)creates logic
	void setDef(const EntityDef& def);
	void setDef(const util::Str8& def_name);
	
	const EntityDef* getDef() const { return m.definition; }
	
	EntityLogic* getLogic(){ return m.logic.get(); }
	const EntityLogic* getLogic() const { return m.logic.get(); }
	
	/// Applies current logic to new def (position etc. doesn't change)
	void changeDef(const EntityDef& def);
	void changeDef(const util::Str8& def_name);
	
	void clear();
	
	/// Copies other.logic by value
	void apply(const Entity& other);

	/// Calls corresponding methods in logic
	
	void setLayer(Layer l);
	Layer getLayer() const;

	void setTransform(const Transform& t_);
	const Transform& getTransform() const;
	
	void setScale(const Transform::Scale& s){ setTransform(Transform(s, getTransform().rotation, getTransform().translation)); }
	void setRotation(const Transform::Rotation& r){ setTransform(Transform(getTransform().scale, r, getTransform().translation)); }
	void setPosition(const Transform::Translation& p){ setTransform(Transform(getTransform().scale, getTransform().rotation, p)); }
	
	Transform::Scale getScale() const { return getTransform().scale; }
	Transform::Rotation getRotation() const { return getTransform().rotation; }
	Transform::Translation getPosition() const { return getTransform().translation; }

	void setDrawPriority(DrawPriority p);
	DrawPriority getDrawPriority() const;
	
	void setCoordinateSpace(util::Coord::Type t);
	util::Coord::Type getCoordinateSpace() const;
	
	void setScaleCoordinateSpace(util::Coord::Type t);
	util::Coord::Type getScaleCoordinateSpace() const;
	
private:
	void informActivation(bool active);
	void onSpatialChange(const Transform& prev_t);
	void onCoordSpaceChange(util::Coord::Type t, util::Coord::Type s);

	void resetDefChangeListening();
	
protected:
	struct {
		bool active;
		Layer layer;
		const EntityDef* definition;
		
		util::CbListener<util::OnChangeCb> definitionChangeListener;
		util::UniquePtr<EntityLogic> logic;
	} m; // To make move cleaner	
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITY_HPP
