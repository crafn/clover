#include "armaturesuit.hpp"
#include "animation/armature.hpp"
#include "util/time.hpp"

namespace clover {
namespace physics {

void ArmatureSuit::update(	const util::SrtTransform3d& transform,
							const animation::ArmaturePose& pose){
	ensure(!pose.isInvalid());
	if (armature == nullptr || armature != &pose.getArmature())
		createObjects(transform, pose);

	sample.transform= transform;
	sample.globalPose= pose.getGlobalPose();

	updateEstimations();
}

RigidObject& ArmatureSuit::get(const util::Str8& joint) const {
	auto it= objects.find(joint);
	if (it == objects.end())
		throw global::Exception("Joint not found: %s", joint.cStr());
	else
		return it->second.ref();
}

void ArmatureSuit::clear(){
	objects.clear();
	armature= nullptr;
}

void ArmatureSuit::preStepUpdates(real64 dt){
	ensure(dt > 0.0);
	for (auto& inst : getInstances())
		inst->preStepUpdate(dt);
}

void ArmatureSuit::preStepUpdate(real64 dt){
	if (!armature || sample.globalPose.empty())
		return;

	const auto& transform= sample.transform;
	const auto& joints= sample.globalPose;

	// util::Set velocities so that objects will move where they should be
	// according to pose
	for (auto& pair : objects){
		const util::Str8& name= pair.first;
		RigidObject& ob= pair.second.ref();

		animation::JointId id= armature->getJointId(name);
		if (id == animation::JointIdNone)
			throw global::Exception("Joint not found: %s", name.cStr());

		util::RtTransform2d prev_t= ob.getTransform();
		ensure(id < joints.size());
		auto t_3d= commonReplaced(util::SrtTransform3d{}, joints[id].transform)*transform;
		util::RtTransform2d cur_t= commonReplaced(util::RtTransform2d{}, t_3d);
		util::RtTransform2d dif_t= cur_t*prev_t.inversed();

		ob.setVelocity((t_3d.translation.xy() - prev_t.translation)*(1.0/dt));
		ob.setAngularVelocity((t_3d.rotation.rotationZ() - prev_t.rotation)*(1.0/dt));
	}
}

void ArmatureSuit::createObjects(const util::SrtTransform3d& t, const animation::ArmaturePose& pose){
	clear();
	armature= &pose.getArmature();
	auto globalpose= pose.getGlobalPose();

	RigidObjectDef def;
	def.setType(RigidObjectType::Kinematic);
	for (const animation::Joint& joint : armature->getJoints()){
		auto t= commonReplaced(
				util::RtTransform2d{},
				globalpose[joint.getId()].transform); 
		def.setTransform(t);
		objects.insert(makePair(joint.getName(),
								util::makeUniquePtr<RigidObject>(RigidObjectDef{t.translation})
		));
	}
}

void ArmatureSuit::updateEstimations(){
	ensure(armature);
	
	const auto& transform= sample.transform;
	const auto& joints= sample.globalPose;

	// Adjust visual positions of objects to match pose
	for (auto& pair : objects){
		const util::Str8& name= pair.first;
		RigidObject& ob= pair.second.ref();

		animation::JointId id= armature->getJointId(name);
		if (id == animation::JointIdNone)
			throw global::Exception("Joint not found: %s", name.cStr());

		ensure(id < joints.size());
		auto t_joint=	commonReplaced(	util::SrtTransform3d{},
										joints[id].transform)*
						transform;
		auto t_ob= commonReplaced(util::SrtTransform3d{}, ob.getTransform());

		auto t_dif= t_joint*t_ob.inversed();
		auto t_est_offset= commonReplaced(
				util::RtTransform2d{}, t_dif);
		auto t_offset= t_dif*commonReplaced(
				util::SrtTransform3d{}, t_est_offset).inversed();

		// 3d offset contains 3d rotation and z-offset
		ob.set3dOffset(t_offset);
		// Estimated transform contains correction to actual position
		// of the physics object, because physics are simulated before
		// game logic and that causes a one-frame delay
		ob.setEstimatedTransform(t_est_offset*ob.getTransform());
	}
}

} // nodes
} // clover
