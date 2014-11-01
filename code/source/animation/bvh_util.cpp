#include "bvh_util.hpp"
#include "debug/debugprint.hpp"
#include "global/exception.hpp"
#include "util/hashmap.hpp"
#include "util/linkedlist.hpp"
#include "util/map.hpp"
#include "util/string.hpp"
#include "global/file.hpp"
#include "global/file_stream.hpp"

#include <string>
#include <sstream>

namespace clover {
namespace animation {

util::Map<util::Str8, BvhJoint::Channel> bvhChannelsByName{
	{"Xposition", BvhJoint::Channel::PosX},
	{"Yposition", BvhJoint::Channel::PosY},
	{"Zposition", BvhJoint::Channel::PosZ},
	{"Xrotation", BvhJoint::Channel::RotX},
	{"Yrotation", BvhJoint::Channel::RotY},
	{"Zrotation", BvhJoint::Channel::RotZ}
};

BvhJoint::Channel bvhChannelByName(const util::Str8& name){
	auto it= bvhChannelsByName.find(name);
	if (it != bvhChannelsByName.end())
		return it->second;
	
	throw global::Exception("Invalid BVH channel name: %s", name.cStr());
}

BvhJoint* findSuperBvhJoint(const BvhJoint& joint, BvhJoint& root){
	if (&joint == &root)
		return &root;
	
	for (BvhJoint& j : root.subJoints){
		BvhJoint* found_joint= findSuperBvhJoint(joint, j);
		if (found_joint)
			return found_joint;
		
	}
	return nullptr;
}

JointPose::Transform localTransformFromBvhValues(util::Vec3f pos, util::Vec3f rot){
	using Quat= JointPose::Transform::Rotation;
	
	real32 deg_to_rad= 2.0*util::pi/360.0;
	Quat rotx= Quat::byRotationAxis({1.0, 0.0, 0.0}, rot.x*deg_to_rad);
	Quat roty= Quat::byRotationAxis({0.0, 1.0, 0.0}, rot.y*deg_to_rad);
	Quat rotz= Quat::byRotationAxis({0.0, 0.0, 1.0}, rot.z*deg_to_rad);
	
	return JointPose::Transform(	1.0f,
									roty*rotx*rotz, // Order Y*X*Z is defined by bvh spec
									pos);
}

util::DynArray<ArmaturePose::Pose> initializedPoses(SizeType joint_count, SizeType pose_count){
	util::DynArray<ArmaturePose::Pose> poses;
	
	ArmaturePose::Pose init_pose;
	init_pose.resize(joint_count);
	for (JointId i= 0; i < joint_count; ++i)
		init_pose[i].jointId= i;
	poses.insert(poses.begin(), pose_count, init_pose);
	
	return poses;
}

/// Collapses BvhJoint tree to an array
/// The order will be same than in sample data
util::DynArray<std::reference_wrapper<const BvhJoint>> bvhJointTreeAsArray(const BvhJoint& root){
	util::DynArray<std::reference_wrapper<const BvhJoint>> array;
	
	std::function<void (const BvhJoint& root)> recursive_add=
	[&] (const BvhJoint& root){
		array.pushBack(std::cref(root));
		for (const BvhJoint& joint : root.subJoints){
			recursive_add(joint);
		}
	};
	recursive_add(root);
	
	return array;
}

void parseBvhDefinition(std::stringstream& stream, BvhData& parsed){
	std::string token;

	int32 indent= 0;
	BvhJoint* current_joint= nullptr;
	
	// Parse up to sample data
	while(stream.good()){
		stream >> token;

		if (token == "{"){
			indent++;
		}
		else if (token == "ROOT"){
			stream >> token;
			parsed.rootJoint.name= token;
			current_joint= &parsed.rootJoint;
		}
		else if (token == "JOINT"){
			if (current_joint == nullptr)
				throw global::Exception("JOINT before ROOT");	
			
			current_joint->subJoints.pushBack(BvhJoint());
			current_joint= &current_joint->subJoints.back();
			
			stream >> token;
			current_joint->name= token;
		}
		else if (token == "OFFSET"){
			if (current_joint == nullptr)
				throw global::Exception("OFFSET before ROOT");
				
			stream >> current_joint->offset[0] >> current_joint->offset[1] >> current_joint->offset[2];
		}
		else if (token == "CHANNELS"){
			if (current_joint == nullptr)
				throw global::Exception("CHANNELS before ROOT");
			
			int32 chan_count;
			stream >> chan_count;

			if (chan_count > 6)
				throw global::Exception("Too many channels");

			for(int32 i= 0; i < chan_count; ++i){
				stream >> token; // Channel name
				current_joint->channels.pushBack(bvhChannelByName(token));
			}
		}
		else if (token == "}"){
			if (current_joint == nullptr)
				throw global::Exception("} before ROOT");
				
			indent--;
			
			current_joint= findSuperBvhJoint(*current_joint, parsed.rootJoint);
			
			if (indent == -1)
				throw global::Exception("Too many }");
		}
		else if (token == "End"){
			if (current_joint == nullptr)
				throw global::Exception("End before ROOT");
			
			current_joint->subJoints.pushBack(BvhJoint());
			current_joint->subJoints.back().name= current_joint->name + "_end"; // Bvh doesn't support explicit end site names
			current_joint= &current_joint->subJoints.back();
			
			
			stream 	>> token >> token >> token // "Site { OFFSET"
					>> current_joint->offset[0]
					>> current_joint->offset[1]
					>> current_joint->offset[2]
					>> token; // "}"
		}
		else if (token == "Frames:"){
			stream >> parsed.frameCount;
		}
		else if (token == "Frame"){
			stream 	>> token // "Time:"
					>> parsed.frameTime;
			break;
		}
	}
}

void parseBvhSampleData(std::stringstream& stream, BvhData& parsed){
	while(stream.good()){
		real32 sample;
		stream >> sample;
		parsed.samples.pushBack(sample);
	}
}

BvhData parseBvhAnimation(const util::Str8& data){
	BvhData parsed;
	
	std::stringstream stream(data.cStr());
	parseBvhDefinition(stream, parsed);
	parseBvhSampleData(stream, parsed);
	
	return parsed;
}

/// Extracts transforms from BvhData
class BvhExtractor {
public:
	BvhExtractor(const BvhData& data_)
			: data(data_)
			, joints(std::move(bvhJointTreeAsArray(data.rootJoint))) {};
	
	JointPose::Transform nextLocalTransform(){
		util::Vec3f raw_rot;
		util::Vec3f raw_pos;
		
		const SizeType channel_count= getJoint().channels.size();
		ensure(channel_count != 0);
		
		SizeType read_samples= 0;
		while (good() && read_samples < channel_count){
			switch (getChannel()){
				case BvhJoint::Channel::PosX: raw_pos.x= nextSample(); break;
				case BvhJoint::Channel::PosY: raw_pos.y= nextSample(); break;
				case BvhJoint::Channel::PosZ: raw_pos.z= nextSample(); break;
				case BvhJoint::Channel::RotX: raw_rot.x= nextSample(); break;
				case BvhJoint::Channel::RotY: raw_rot.y= nextSample(); break;
				case BvhJoint::Channel::RotZ: raw_rot.z= nextSample(); break;
				default: throw global::Exception("Invalid channel");
			}
			
			++read_samples;
		}
		
		return localTransformFromBvhValues(raw_pos, raw_rot);
	}
	
	bool good() const { return frameI < data.frameCount; }
	
	const BvhJoint& getJoint() const {
		ensure(jointI < joints.size());
		return joints[jointI];
	}
	
	SizeType getJointCount() const {
		return joints.size();
	}
	
	SizeType getFrameIndex() const { return frameI; }
	
private:

	BvhJoint::Channel getChannel(){
		ensure(chI < getJoint().channels.size());
		return getJoint().channels[chI];
	}
	
	real32 nextSample(){
		ensure(sampleI < data.samples.size());
	
		real32 sample= data.samples[sampleI];
		++sampleI;
		++chI;
		
		// Loop until increments for indices are done
		bool incremented;
		do {
			incremented= false;
			if (chI == getJoint().channels.size()){
				// All samples read for a joint transform
				incrementJoint();
				incremented= true;
			}
			
			if (jointI == joints.size()){
				// All joint transforms read for a frame
				incrementFrame();
				incremented= true;
			}
		} while (incremented && frameI < data.frameCount);
		
		return sample;
	}
	
	void incrementJoint(){
		++jointI;
		chI= 0;
	}
	
	void incrementFrame(){
		++frameI;
		jointI= 0;
		chI= 0;
	};
	
	const BvhData& data;

	SizeType frameI= 0;
	SizeType jointI= 0;
	SizeType chI= 0;
	SizeType sampleI= 0;
	
	util::DynArray<std::reference_wrapper<const BvhJoint>> joints;
};

util::DynArray<ArmaturePose::Pose> calcLocalPosesFromBvh(const BvhData& bvh_data, const util::HashMap<util::Str8, JointId>& joint_name_to_id){
	BvhExtractor extractor(bvh_data);
	util::DynArray<ArmaturePose::Pose> poses= initializedPoses(extractor.getJointCount(), bvh_data.frameCount);
	
	auto pose_i= [&] () -> SizeType {
		SizeType f= extractor.getFrameIndex();
		ensure(f < poses.size());
		return f;
	};
	
	auto joint_i= [&] () -> JointId  {
		JointId id= joint_name_to_id.get(extractor.getJoint().name, JointIdNone);
		
		if (id == JointIdNone)
			throw global::Exception("Invalid joint name: %s", extractor.getJoint().name.cStr());
		
		ensure(id < extractor.getJointCount());
		
		return id;
	};
	
	while (extractor.good()){
		auto& pose= poses[pose_i()][joint_i()];
		pose.transform= extractor.nextLocalTransform();
	}
	
	return poses;
}

} // animation
} // clover