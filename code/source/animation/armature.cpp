#include "armature.hpp"
#include "assimp_util.hpp"

namespace clover {
namespace animation {
	
//
// JointDef util
//

JointId jointIdByName(const util::Str8& name, const util::DynArray<JointDef>& joint_defs){
	for (SizeType i= 0; i < joint_defs.size(); ++i){
		if (joint_defs[i].name == name)
			return (JointId)i;
	}
	return JointIdNone;
}

JointPose jointPoseByName(const util::Str8& name, const util::DynArray<JointDef>& joint_defs){
	JointId joint_id= jointIdByName(name, joint_defs);
	if (joint_id == JointIdNone)
		throw global::Exception("Joint not found: %s", name.cStr());
	
	JointPose pose;
	pose.jointId= joint_id;
	pose.transform= joint_defs[joint_id].transform;
	return pose;
}

util::DynArray<Joint> createJoints(const util::DynArray<JointDef>& joint_defs){
	release_ensure_msg(joint_defs.size() <= maxJointCount, "Too many joints (max: %i)", maxJointCount);
	
	util::DynArray<Joint> joints;
	for (SizeType i= 0; i < joint_defs.size(); ++i){
		const JointDef& def= joint_defs[i];

		joints.emplaceBack(	def.name,
							jointPoseByName(def.name, joint_defs),
							(JointId)i,
							jointIdByName(def.superJointName, joint_defs));
	}
	
	return joints;
}

//
// Joint util
//

ArmaturePose makeBindPose(const Armature& armature){
	const Armature::Joints& joints= armature.getJoints();
	ArmaturePose::Pose pose(joints.size());
	for (SizeType i= 0; i < joints.size(); ++i){
		pose[i]= joints[i].getBindPose();
	}
	return ArmaturePose(armature, pose);
}

//
// Armature
//

Armature::Armature()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(jointDefsAttribute, "joints", {})
		, INIT_RESOURCE_ATTRIBUTE(importAttribute, "import", "")
		, bindPose(makeBindPose(*this)){

	jointDefsAttribute.setOnChangeCallback([this] () {
		if (getResourceState() == State::Loaded)
			setResourceState(State::Unloaded); // Reload
	});
}

JointId Armature::getJointId(const util::Str8& name) const {
	for (const auto& joint : joints){
		if (joint.getName() == name)
			return joint.getId();
	}
	throw resources::ResourceException("Joint not found: %s", name.cStr());
}

JointId Armature::findJointId(const util::Str8& name) const {
	for (const auto& joint : joints){
		if (joint.getName() == name)
			return joint.getId();
	}
	return JointIdNone;
}

void Armature::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		try {
			print(	debug::Ch::Anim, debug::Vb::Trivial,
					"Loading armature: %s", nameAttribute.get().cStr());
			if (importAttribute.get().relative().empty()){
				joints= createJoints(jointDefsAttribute.get());
			}
			else {
				util::Str8 path= importAttribute.get().whole();
				joints= createJoints(jointDefsFromAssimp(parseAssimp(path)));
			}
			bindPose= makeBindPose(*this);
			setResourceState(State::Loaded);
		}
		catch (const global::Exception& e){
			setResourceState(State::Error);
		}
	}
	else {
		joints.clear();
		bindPose= makeBindPose(*this);
		setResourceState(State::Unloaded);
	}
}

void Armature::createErrorResource(){
	joints.clear();
	bindPose= makeBindPose(*this);
	setResourceState(State::Error);
}

JointNameToIdMap jointNameToIdMap(const Armature& a){
	JointNameToIdMap map;
	for (const auto& joint : a.getJoints()){
		map[joint.getName()]= joint.getId();
	}
	return map;
}

util::DynArray<JointId> jointIdMapForArmatures(const Armature& from, const Armature& to){
	util::DynArray<JointId> map(from.getJoints().size());
	
	if (&from == &to){
		for (SizeType i= 0; i < map.size(); ++i)
			map[i]= i;
	}
	else {
		auto to_map= jointNameToIdMap(to);
		
		for (const auto& joint : from.getJoints()){
			map[joint.getId()]= to_map[joint.getName()];
		}
	}
	return map;
}

} // animation
} // clover