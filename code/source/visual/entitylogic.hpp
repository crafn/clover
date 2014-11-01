#ifndef CLOVER_VISUAL_ENTITY_INSTANCELOGIC_HPP
#define CLOVER_VISUAL_ENTITY_INSTANCELOGIC_HPP

#include "build.hpp"
#include "entity_def.hpp"
#include "util/coord.hpp"
#include "util/transform.hpp"

namespace clover {
namespace visual {

/// Actual rendering info of visual::Entity
/// EntityLogic is recreated if definition changes
/// @todo Consider renaming to *EntityInstance or *EntityCfg
class EntityLogic {
public:
	using Transform= util::SrtTransform<util::Vec3d, util::Quatd, util::Vec3d>;
	using DrawPriority= EntityDef::DrawPriority;
	
	enum class Layer {
		World,
		PhysicsDebug,
		Gui,
		Debug
	};
	
	EntityLogic(const EntityDef& def);
	virtual ~EntityLogic();
	
	const EntityDef& getDef() const;

	virtual void setTransform(const Transform& t_){ transform= t_; }
	const Transform& getTransform() const { return transform; }
	
	void setScale(const Transform::Scale& s){ setTransform(Transform(s, transform.rotation, transform.translation)); }
	void setRotation(const Transform::Rotation& r){ setTransform(Transform(transform.scale, r, transform.translation)); }
	void setPosition(const Transform::Translation& p){ setTransform(Transform(transform.scale, transform.rotation, p)); }
	
	Transform::Scale getScale() const { return transform.scale; }
	Transform::Rotation getRotation() const { return transform.rotation; }
	Transform::Translation getPosition() const { return transform.translation; }
	
	
	/// Currently works only for models
	virtual void setDrawPriority(DrawPriority p){}
	virtual DrawPriority getDrawPriority() const { return 0; }
	
	void setLayer(Layer l){ layer= l; }
	Layer getLayer() const { return layer; }
	
	void setCoordinateSpace(util::Coord::Type t){ coordinateSpace= t; }
	util::Coord::Type getCoordinateSpace() const { return coordinateSpace; }
	
	void setScaleCoordinateSpace(util::Coord::Type t){ scaleCoordinateSpace= t; }
	util::Coord::Type getScaleCoordinateSpace() const { return scaleCoordinateSpace == util::Coord::None ? coordinateSpace : scaleCoordinateSpace; }
	
	/// Copies properties of 'other' without copying definition or causing slicing
	virtual void apply(const EntityLogic& other);
	
	/// Don't use if EntityLogic is inside a visual::Entity! Will mess up cache in EntityMgr
	void setDef(const EntityDef& def);
	
protected:
	Transform transform;
	
private:
	const EntityDef* definition;
	Layer layer;
	util::Coord::Type coordinateSpace;
	util::Coord::Type scaleCoordinateSpace;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITY_INSTANCELOGIC_HPP