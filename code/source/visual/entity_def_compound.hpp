#ifndef CLOVER_VISUAL_ENTITY_DEF_COMPOUND_HPP
#define CLOVER_VISUAL_ENTITY_DEF_COMPOUND_HPP

#include "build.hpp"
#include "entity_def.hpp"
#include "entitylogic_compound.hpp"
#include "util/math.hpp"

namespace clover {
namespace visual {

class CompountEntityDef;

} // visual
namespace resources {

template <>
struct ResourceTraits<visual::CompoundEntityDef> {
	DECLARE_RESOURCE_TRAITS(visual::CompoundEntityDef, String)
	
	typedef ResourceTraits<visual::EntityDef> BaseTraits;
	
	static util::DynArray<AttributeDef> getAttributeDefs(){
		return	BaseTraits::getAttributeDefs().pushBacked(util::DynArray<AttributeDef> {
						AttributeDef::Resource("armature")
				});
	}
	
	typedef VisualDerivedEntityDefSubCache<visual::CompoundEntityDef> SubCacheType;
	
	static util::Str8 typeName(){ return "CompoundEntityDef"; }
	static util::Str8 identifierKey(){ return "name"; }
	
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static bool expired(const visual::CompoundEntityDef& def){ return false; }
};

} // resources
namespace visual {

/// Definition for an entity which consist of armature and other entities
class CompoundEntityDef : public EntityDef {
public:
	DECLARE_RESOURCE(CompoundEntityDef)
	
	CompoundEntityDef();
	virtual ~CompoundEntityDef(){}
	
	const animation::Armature& getArmature() const;
	const util::DynArray<ArmatureAttachmentDef>& getAttachmentDefs() const;
	
	virtual CompoundEntityLogic* createLogic() const override;

	virtual void resourceUpdate(bool load, bool force= true) override;
	virtual void createErrorResource() override;
	
private:
	
	RESOURCE_ATTRIBUTE(Resource, armatureAttribute)
	RESOURCE_ATTRIBUTE(ArmatureAttachmentDefArray, attachmentsAttribute)
	
	util::CbListener<util::OnChangeCb> armatureChangeListener;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITY_DEF_COMPOUND_HPP