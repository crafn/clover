#ifndef CLOVER_VISUAL_ENTITY_DEF_HPP
#define CLOVER_VISUAL_ENTITY_DEF_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "util/boundingbox.hpp"
#include "util/transform.hpp"
#include "util/vector.hpp"

namespace clover {
namespace visual {

class EntityDef;

} // visual
namespace resources {

class VisualEntityDefSubCache;

template <>
struct ResourceTraits<visual::EntityDef> {
	DECLARE_RESOURCE_TRAITS(visual::EntityDef, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"));
	
	typedef VisualEntityDefSubCache SubCacheType;
	
	static util::Str8 typeName(){ return "VisualEntityDef"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::EntityDef& def){ return false; }
};

} // resources
namespace visual {

class EntityLogic;

/// Entity definition
class EntityDef : public resources::Resource {
public:
	DECLARE_RESOURCE(EntityDef)

	using Transform= util::SrtTransform3d;
	
	/// Priority when drawing things on ~same depth
	/// Larger values are drawn in front of smaller
	typedef int32 DrawPriority;
	
	enum class Type {
		None,
		Model,
		Light,
		ParticleSource,
		Compound
	};
	
	EntityDef();
	EntityDef(const EntityDef&)= default;
	EntityDef(EntityDef&&)= default;
	
	EntityDef& operator=(const EntityDef&)= default;
	EntityDef& operator=(EntityDef&&)= default;

	virtual ~EntityDef();
	
	Type getType() const;
	
	void setOffset(Transform t){ offsetAttribute.set(t); }
	const Transform& getOffset() const { return offsetAttribute.get(); }
	
	virtual EntityLogic* createLogic() const;
	
	virtual void resourceUpdate(bool load, bool force=true);
	virtual void createErrorResource();
	
protected:
	Type type;
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(SrtTransform3, offsetAttribute)
};

} // visual
} // clover

#include "subcache_entity.hpp"

#endif // CLOVER_VISUAL_ENTITY_DEF_HPP