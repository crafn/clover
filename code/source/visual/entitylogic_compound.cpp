#include "entitylogic_compound.hpp"
#include "entity_def_compound.hpp"
#include "resources/cache.hpp"
#include "util/math.hpp"

namespace clover {
namespace visual {

CompoundEntityLogic::CompoundEntityLogic(const CompoundEntityDef& def)
		: Base(def)
		, definition(&def)
		, armature(&def.getArmature())
		, pose(armature->getBindPose()){
	for (const auto& attachment_def : getDef().getAttachmentDefs()){
		attachedEntities.pushBack(std::move(createAttachedEntity(attachment_def, *armature)));
	}
}

const CompoundEntityDef& CompoundEntityLogic::getDef() const {
	ensure(definition);
	return *definition;
}

void CompoundEntityLogic::setPose(animation::ArmaturePose armature_pose){
	pose= std::move(armature_pose);
	// Update entity positionsä
	setTransform(getTransform());
}

void CompoundEntityLogic::setTransform(const Base::Transform& t){
	Base::setTransform(t);
	
	const animation::ArmaturePose::Pose& joint_poses= pose.getGlobalPose();
	
	// Allow different source armatures for pose
	// This could be cached
	const auto& arm_id_to_pose_id= animation::jointIdMapForArmatures(*armature, pose.getArmature());
	
	// Transforming entities by offset and joint transform
	for (auto& m : attachedEntities){
		ensure(m.jointId < arm_id_to_pose_id.size());
		
		animation::JointId pose_joint_id= arm_id_to_pose_id[m.jointId];
		if (pose_joint_id >= joint_poses.size()){
			print(debug::Ch::Visual, debug::Vb::Moderate,
					"Pose has invalid joint id for armature %s: %i",
					armature->getName().cStr(),
					pose_joint_id);
			continue;
		}
		
		animation::JointPose::Transform pose_t= m.offset*joint_poses[pose_joint_id].transform;
		using PoseRot= animation::JointPose::Transform::Rotation;
		
		Entity::Transform casted_pose_t; // Entity transform in armature coordinates
		casted_pose_t.scale= Entity::Transform::Scale(pose_t.scale);
		casted_pose_t.rotation= pose_t.rotation.casted<Entity::Transform::Rotation>();
		casted_pose_t.translation= pose_t.translation.casted<Entity::Transform::Translation>();
		
		m.entity->setTransform(casted_pose_t*getDef().getOffset()*transform);
	}
}

void CompoundEntityLogic::onSetActive(bool b){
	for (auto& m : attachedEntities){
		m.entity->setActive(b);
	}
}

util::DynArray<Entity*> CompoundEntityLogic::getEntities() const {
	util::DynArray<Entity*> entities;
	for (auto&& e : attachedEntities)
		entities.pushBack(&(*e.entity));
	return entities;
}

auto CompoundEntityLogic::createAttachedEntity(const ArmatureAttachmentDef& def, const animation::Armature& armature) -> AttachedEntity {
	AttachedEntity e;
	e.entity= EntityPtr(new Entity(resources::gCache->getResource<visual::EntityDef>(def.entityName)));
	e.jointId= armature.getJointId(def.jointName);
	if (def.offset)
		e.offset= *def.offset;
	return e;
}

} // visual
} // clover
