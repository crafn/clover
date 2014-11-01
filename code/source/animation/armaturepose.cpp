#include "armaturepose.hpp"
#include "armature.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace animation {

/// Returns transform of super joint, or identity transform if there's no super joint
JointPose::Transform superJointTransform(JointId id, const Armature::Joints& joints, const ArmaturePose::Pose& pose){
	auto super_id= joints[id].getSuperId();
	
	if (super_id == JointIdNone)
		return JointPose::Transform();
	
	ensure(super_id < joints.size());
	ensure(super_id < pose.size());
	return pose[super_id].transform;
}

/// Calculates global pose of a joint by local-in-bind pose
JointPose::Transform globalJointTransform(JointId id, const Armature::Joints& joints, const ArmaturePose::Pose& local_in_bind_pose){
	ensure(id != JointIdNone);
	ensure(id < local_in_bind_pose.size());
	ensure(id < joints.size());
	
	// Transform joint to shape of bind pose
	JointPose::Transform t= local_in_bind_pose[id].transform*joints[id].getBindPose().transform;
	JointId super_id= joints[id].getSuperId();
	
	if (super_id != JointIdNone)
		t *= globalJointTransform(super_id, joints, local_in_bind_pose);
	
	return t;
}

ArmaturePose::Pose globalPoseFromLocalInBindPose(const Armature::Joints& joints, const ArmaturePose::Pose& local_in_bind_pose){
	ensure(joints.size() == local_in_bind_pose.size());
	
	ArmaturePose::Pose pose(joints.size());
	for (SizeType i= 0; i < joints.size(); ++i){
		ensure(i < pose.size());
		pose[i].jointId= i;
		pose[i].transform= globalJointTransform(i, joints, local_in_bind_pose);
		ensure(isFinite(pose[i].transform));
	}
	
	return pose;
}

ArmaturePose ArmaturePose::byGlobalPose(const Armature& armature, Pose global_pose){
	Pose inbind_pose= global_pose;

	/// @fixme Naming: getLocalInBindPose -> getLocalPose
	const Pose& local_bind_pose= armature.getBindPose().getLocalInBindPose();
	ensure(inbind_pose.size() == local_bind_pose.size());
	
	// Calculate local-in-bind -pose from global pose
	for (JointId i= 0; i < inbind_pose.size(); ++i){
		JointPose::Transform super_t= superJointTransform(i, armature.getJoints(), global_pose);
		
		ensure(i < inbind_pose.size() && i < global_pose.size() && i < local_bind_pose.size());
		inbind_pose[i].transform= global_pose[i].transform*super_t.inversed()*local_bind_pose[i].transform.inversed();
		ensure(isFinite(inbind_pose[i].transform));
	}
	
	return ArmaturePose(armature, inbind_pose);
}

ArmaturePose ArmaturePose::byLocalPose(const Armature& armature, Pose local_pose){
	Pose inbind_pose= local_pose;

	/// @fixme Naming: getLocalInBindPose -> getLocalPose
	const Pose& local_bind_pose= armature.getBindPose().getLocalInBindPose();
	ensure(inbind_pose.size() == local_bind_pose.size());

	// Calculate local-in-bind -pose from local pose
	for (JointId i= 0; i < inbind_pose.size(); ++i){
		ensure(i < inbind_pose.size() && i < local_pose.size() && i < local_bind_pose.size());
		inbind_pose[i].transform= local_pose[i].transform*local_bind_pose[i].transform.inversed();
		ensure(isFinite(inbind_pose[i].transform));
	}

	return ArmaturePose(armature, inbind_pose);
}

ArmaturePose::ArmaturePose()
		: armature(&resources::gCache->getErrorResource<Armature>())
		, localInBindPose(defaultPose()){
}

ArmaturePose::ArmaturePose(const Armature& armature_, Pose local_pose_)
		: armature(&armature_)
		, localInBindPose(std::move(local_pose_)){
	ensure(getArmature().getJoints().size() == localInBindPose.size());
}

const Armature& ArmaturePose::getArmature() const {
	ensure(armature);
	return *armature;
}

ArmaturePose::Pose ArmaturePose::getGlobalPose() const {
	// This could be cached
	return globalPoseFromLocalInBindPose(getArmature().getJoints(), localInBindPose);
}

void ArmaturePose::setArmature(const util::Str8& name){
	armature= &resources::gCache->getResource<Armature>(name);
	localInBindPose= defaultPose();
}

const util::Str8& ArmaturePose::getArmatureName() const {
	return getArmature().getName();
}

ArmaturePose::Pose ArmaturePose::defaultPose() const {
	auto pose= getArmature().getBindPose().getLocalInBindPose();
	for (auto& joint : pose){
		joint.transform= JointPose::Transform();
	}
	return pose;
}

} // animation

animation::ArmaturePose::Pose lerp(	const animation::ArmaturePose::Pose& pose1,
									const animation::ArmaturePose::Pose& pose2,
									real64 f){
	if (pose1.size() != pose2.size())
		throw global::Exception("Joint counts don't match: %i != %i", pose1.size(), pose2.size());
	
	SizeType size= pose1.size();
	animation::ArmaturePose::Pose result(size);
	
	for (SizeType i= 0; i < size; ++i){
		if (pose1[i].jointId != pose2[i].jointId)
			throw global::Exception("Joint ids doesn't match: %i != %i", pose1[i].jointId, pose2[i].jointId);
		
		result[i].jointId= pose1[i].jointId;
		result[i].transform= lerp(pose1[i].transform, pose2[i].transform, f);
	}
	
	return result;
}

} // clover