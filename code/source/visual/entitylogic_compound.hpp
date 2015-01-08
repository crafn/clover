#ifndef CLOVER_VISUAL_ENTITYLOGIC_COMPOUND_HPP
#define CLOVER_VISUAL_ENTITYLOGIC_COMPOUND_HPP

#include "animation/armature.hpp"
#include "animation/armaturepose.hpp"
#include "animation/joint.hpp"
#include "build.hpp"
#include "entity.hpp"
#include "entitylogic.hpp"

namespace clover {
namespace visual {

class CompoundEntityDef;

class CompoundEntityLogic :	 public EntityLogic {
public:
	using Base= EntityLogic;

	CompoundEntityLogic(const CompoundEntityDef& def);
	virtual ~CompoundEntityLogic(){}
	
	const CompoundEntityDef& getDef() const;
	
	void setPose(animation::ArmaturePose armature_pose);
	
	virtual void setTransform(const Base::Transform& t) override;
	
	void onSetActive(bool b);
	
	util::DynArray<Entity*> getEntities() const;

private:
	using EntityPtr= util::UniquePtr<Entity>;
	struct AttachedEntity {
		EntityPtr entity;
		animation::JointId jointId;
		ArmatureAttachmentDef::Transform offset;
	};

	static AttachedEntity createAttachedEntity(const ArmatureAttachmentDef& def, const animation::Armature& armature);
	
	/// @todo Remove and use casted base class def ptr
	const CompoundEntityDef* definition;
	
	const animation::Armature* armature;
	animation::ArmaturePose pose;
	util::DynArray<AttachedEntity> attachedEntities;
};

} // visual
} // clover

#endif // CLOVER_VISUAL_ENTITYLOGIC_COMPOUND_HPP
