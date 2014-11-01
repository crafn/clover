#ifndef CLOVER_VISUAL_ENTITY_DEF_LIGHT_HPP
#define CLOVER_VISUAL_ENTITY_DEF_LIGHT_HPP

#include "build.hpp"
#include "entity_def.hpp"
#include "entitylogic_light.hpp"
#include "util/math.hpp"

namespace clover {
namespace visual {

class LightEntityDef;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::LightEntityDef> {
	DECLARE_RESOURCE_TRAITS(visual::LightEntityDef, String)
	
	typedef ResourceTraits<visual::EntityDef> BaseTraits;
	
	static util::DynArray<AttributeDef> getAttributeDefs(){
		return 	std::move(
					BaseTraits::getAttributeDefs().pushBacked(util::DynArray<AttributeDef> {
						AttributeDef::Real("halfValueRadius"),
						AttributeDef::Real("intensity"),
						AttributeDef::Boolean("shadows")
					})
				);
	}
	
	typedef VisualDerivedEntityDefSubCache<visual::LightEntityDef> SubCacheType;
	
	static util::Str8 typeName(){ return "LightEntityDef"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::LightEntityDef& def){ return false; }
};

} // resources
namespace visual {

/// Definition for a lamp
class LightEntityDef : public EntityDef {
public:
	DECLARE_RESOURCE(LightEntityDef)

	LightEntityDef(real32 dist=1.0, real32 intens=0.1, bool shadows=false);
	virtual ~LightEntityDef();
	
	void setIntensity(real32 f){ intensityAttribute.set(f); }
	real32 getIntensity() const { return intensityAttribute.get(); }

	void setHalfValueRadius(real32 r){ radiusAttribute.set(r); }
	real32 getHalfValueRadius() const { return radiusAttribute.get(); }

	bool hasShadows() const { return shadowsAttribute.get(); }

	virtual LightEntityLogic* createLogic() const;

	virtual void resourceUpdate(bool load, bool force= true);
	virtual void createErrorResource();
	
private:
	RESOURCE_ATTRIBUTE(Real, radiusAttribute)
	RESOURCE_ATTRIBUTE(Real, intensityAttribute)
	RESOURCE_ATTRIBUTE(Boolean, shadowsAttribute)
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITY_DEF_LIGHT_HPP