#include "assimp_util.hpp"
#include "global/exception.hpp"
#include "global/file.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace animation {
namespace detail {

static aiVectorKey& findPreviousPosKeyForTime(real64 t, aiNodeAnim& joint){
	for (SizeType i= joint.mNumPositionKeys; i > 0; --i){
		aiVectorKey& key= joint.mPositionKeys[i - 1];
		if (key.mTime <= t || i - 1 == 0)
			return key;
	}

	release_ensure(0);
}

static aiQuatKey& findPreviousRotKeyForTime(real64 t, aiNodeAnim& joint){
	for (SizeType i= joint.mNumRotationKeys; i > 0; --i){
		aiQuatKey& key= joint.mRotationKeys[i - 1];
		if (key.mTime <= t || i - 1 == 0)
			return key;
	}

	release_ensure(0);
}

static aiVectorKey& findPreviousScaleKeyForTime(real64 t, aiNodeAnim& joint){
	for (SizeType i= joint.mNumScalingKeys; i > 0; --i){
		aiVectorKey& key= joint.mScalingKeys[i - 1];
		if (key.mTime <= t || i - 1 == 0)
			return key;
	}

	release_ensure(0);
}

static JointPose::Transform findPreviousTransformForTime(real64 time, aiNodeAnim& joint){
	JointPose::Transform t;

	if (joint.mNumPositionKeys > 0){
		aiVectorKey& pos_key=
			findPreviousPosKeyForTime(time, joint);
		aiVector3D& pos= pos_key.mValue;

		t.translation= util::Vec3f{pos.x, pos.y, pos.z};
	}

	if (joint.mNumRotationKeys > 0){
		aiQuatKey& rot_key=
			findPreviousRotKeyForTime(time, joint);
		aiQuaternion& rot= rot_key.mValue;

		t.rotation= util::Quatf{rot.x, rot.y, rot.z, rot.w};
	}

	if (joint.mNumScalingKeys > 0){
		aiVectorKey& scale_key=
			findPreviousScaleKeyForTime(time, joint);
		aiVector3D& scale= scale_key.mValue;

		t.scale= scale.x;
	}

	return t;
}

static util::Str8 filterImportedJointName(const char* name){
	// Blender/assimp assigns incorrect names for fbx bones: "ObjName|BoneName"
	// So lets strip "ObjName|" away
	return util::Str8{name}.splitted('|').back();
}

} // detail

using namespace detail;

AssimpData parseAssimp(const util::Str8& path){
	util::Str8 ext= global::File::extension(path);
	util::DynArray<uint8> data= global::File::readAll(path);
	if (data.empty())
		throw global::Exception("Invalid path: %s", path.cStr());

	AssimpData ai;
	ai.importer= util::makeUniquePtr<Assimp::Importer>();
	ai.scene=
		ai.importer->ReadFileFromMemory(
				data.data(),
				data.size(),
				0,
				ext.cStr());

	if (!ai.scene){
		throw global::Exception("Importing animation failed: %s",
				ai.importer->GetErrorString());
	}

	if (ai.scene->mNumAnimations == 0)
		throw global::Exception("Importing animation failed: no animation");
	if (ai.scene->mNumAnimations > 1)
		throw global::Exception(
				"Importing animation failed: multiple animations (%i)",
				(int32)ai.scene->mNumAnimations);

	ai.animation= NONULL(ai.scene->mAnimations[0]);

	if (ai.animation->mDuration <= 0.0)
		throw global::Exception("Importing animation failed: invalid duration: %f",
				ai.animation->mDuration);

	if (ai.animation->mTicksPerSecond <= 0.0)
		throw global::Exception("Importing animation failed: invalid fps: %f",
				ai.animation->mTicksPerSecond);

	if (ai.animation->mNumChannels <= 0)
		throw global::Exception("Importing animation failed: invalid channel count: %i",
				(int32)ai.animation->mNumChannels);

	ai.fps= ai.animation->mTicksPerSecond;
	ai.frameCount= static_cast<SizeType>(round(ai.animation->mDuration*ai.fps));

	ensure(ai.frameCount > 0);

	return ai;
}

util::DynArray<ArmaturePose::Pose> localPosesFromAssimp(
		const AssimpData& ai,
		const util::HashMap<util::Str8, JointId>& joint_name_to_id){

	// Construct default pose which contains all joints needed
	ArmaturePose::Pose default_pose;
	for (const auto& pair : joint_name_to_id){
		JointId id= pair.second;
		if (id >= default_pose.size() && id != JointIdNone){
			default_pose.resize(id + 1);
		}
	}

	aiAnimation& anim= *NONULL(ai.animation);

	util::DynArray<ArmaturePose::Pose> samples;
	for (SizeType sample_i= 0; sample_i < ai.frameCount; ++sample_i){
		ArmaturePose::Pose pose= default_pose;

		for (SizeType joint_i= 0; joint_i < anim.mNumChannels; ++joint_i){
			aiNodeAnim& joint= *NONULL(anim.mChannels[joint_i]);
			util::Str8 joint_name= filterImportedJointName(joint.mNodeName.C_Str());

			// util::Map assimp joint to animation::Joint
			auto joint_name_it= joint_name_to_id.find(joint_name);
			if (	joint_name_it == joint_name_to_id.end() ||
					joint_name_it->second == JointIdNone){
				throw global::Exception(
						"Importing animation failed: unexpected joint: %s",
						joint_name.cStr());
			}
			JointId joint_id= joint_name_it->second;
			ensure(joint_id < pose.size());
			
			pose[joint_id].transform=
					findPreviousTransformForTime(sample_i/ai.fps, joint);
		}
		samples.pushBack(pose);
	}

	ensure(!samples.empty());

	return samples;
}

util::DynArray<JointDef> jointDefsFromAssimp(const AssimpData& ai){
	const aiScene& scene= *NONULL(ai.scene);
	aiAnimation& anim= *NONULL(ai.animation);

	util::DynArray<JointDef> defs;
	for (SizeType joint_i= 0; joint_i < anim.mNumChannels; ++joint_i){
		aiNodeAnim& joint= *NONULL(anim.mChannels[joint_i]);

		JointDef def;
		def.name= filterImportedJointName(joint.mNodeName.C_Str());
		print(debug::Ch::General, debug::Vb::Trivial, "Joint name: %s", def.name.cStr());
		def.transform= findPreviousTransformForTime(0.0, joint);

		aiNode& joint_node= *NONULL(scene.mRootNode->FindNode(joint.mNodeName.C_Str()));
		if (joint_node.mParent){
			def.superJointName= joint_node.mParent->mName.C_Str();
		}

		defs.pushBack(def);
	}
	return defs;
}

} // animation
} // clover