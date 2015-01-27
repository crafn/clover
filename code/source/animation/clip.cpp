#include "clip.hpp"
#include "armature.hpp"
#include "bvh_util.hpp"
#include "global/env.hpp"
#include "global/file.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace animation {

util::DynArray<ArmaturePose::Pose> inBindByLocalSamples(
		const Armature& armature,
		util::DynArray<ArmaturePose::Pose> local_samples){
	util::DynArray<ArmaturePose::Pose> converted_samples=
		std::move(local_samples);
	for (ArmaturePose::Pose& sample : converted_samples){
		sample= ArmaturePose::byLocalPose(armature, sample).
			getLocalInBindPose(); 
	}
	return converted_samples;
}

Clip::Clip()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(fileAttribute, "file", "")
		, INIT_RESOURCE_ATTRIBUTE(armatureAttribute, "armature", "")
		, INIT_RESOURCE_ATTRIBUTE(channels, "channels", {})
		, fps(30.0){
	
	fileAttribute.setOnChangeCallback([&] () {
		unload();
	});
	
	fileAttribute.get().setOnFileChangeCallback([&] (resources::PathAttributeValue::FileEvent f){
		unload();
	});
}

ArmaturePose Clip::getPose(real32 phase, bool interpolate) const {
	phase= util::limited(phase, 0.0f, 1.0f);
	real64 scaled_phase= phase*(samples.size() - 1);
	SizeType sample_i= std::floor(scaled_phase);

	ensure_msg(sample_i >= 0 && sample_i < samples.size(), "samples.size(): %i, sample_i: %i", samples.size(), sample_i);
	
	if (!interpolate || sample_i + 1 == samples.size())
		return ArmaturePose(getArmature(), samples[sample_i]);
	
	double dummy_int_part;
	return ArmaturePose(getArmature(),
						lerp(	samples[sample_i],
								samples[sample_i + 1],
								std::modf(scaled_phase, &dummy_int_part)));
}

const Armature& Clip::getArmature() const {
	return global::g_env.resCache->getResource<Armature>(armatureAttribute.get());
}

void Clip::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		try {
			this->load();
		}
		catch (global::Exception& e){
			createErrorResource();
		}
	}
	else {
		unload();
	}
}

void Clip::createErrorResource(){
	unload();
	setResourceState(State::Error);
}

void Clip::load(){
	print(	debug::Ch::Anim, debug::Vb::Trivial,
			"Loading clip: %s", nameAttribute.get().cStr());

	armatureChangeListener.clear();
	armatureChangeListener.listen(getArmature(), [&] (){
		setResourceState(State::Unloaded); // Reload
	});

	util::Str8 path= fileAttribute.get().whole();
	auto file_contents= global::File::readText(path);
	if (!file_contents.empty()) {
		BvhData bvh_data= parseBvhAnimation(file_contents);
		fps= 1.0/bvh_data.frameTime;
		samples= calcLocalPosesFromBvh(bvh_data, jointNameToIdMap(getArmature()));
	} else { // Custom format loading
		fps= 30.0;
		real32 duration= 0.0;
		for (auto& ch : channels.get()) {
			if (ch.keys.empty())
				continue;
			duration= std::max(ch.keys.back().time, duration);
		}

		auto& armature= getArmature();
		SizeType joint_count= armature.getJoints().size();
		const LocalPose& default_pose= armature.getBindPose().getLocalInBindPose();
		// Looping animation works by having last key == first key. 
		// The one additional sample serves as interp. target for the "last" sample
		samples.resize(std::floor(duration*fps + 0.5) + 1, default_pose);

		// Extract values from keyframes to LocalPose samples
		for (auto& ch : channels.get()) {
			auto joint_id= armature.getJointId(ch.joint);
			if (joint_id == JointIdNone)
				throw resources::ResourceException(
					"animation::Clip invalid joint: %s, %s",
					getName().cStr(), ch.joint.cStr());

			if (ch.keys.empty())
				continue;

			auto next_key_it= ch.keys.begin();
			auto prev_key_it= ch.keys.begin();
			for (SizeType s_i= 0; s_i < samples.size(); ++s_i) {
				real32 sample_time= s_i/fps;
				while (	next_key_it != ch.keys.end() &&
						next_key_it + 1 != ch.keys.end() &&
						next_key_it->time <= sample_time)
					prev_key_it= next_key_it++;

				JointPose::Transform& t= samples[s_i][joint_id].transform;
				real32* prev_v= prev_key_it->value; // 4 floats
				real32* next_v= next_key_it->value; // 4 floats
				real32 frac=
					(sample_time - prev_key_it->time)/
					(next_key_it->time - prev_key_it->time);
				util::clamp(frac, 0.0f, 1.0f);

				switch (ch.type) {
					case ClipChannelType::translation: {
						t.translation[0]= util::lerp(prev_v[0], next_v[0], frac);
						t.translation[1]= util::lerp(prev_v[1], next_v[1], frac);
						t.translation[2]= util::lerp(prev_v[2], next_v[2], frac);
					} break;
					case ClipChannelType::rotation: {
						util::Quatf p= util::Quatf::byRotationAxis(
									{prev_v[0], prev_v[1], prev_v[2]}, prev_v[3]);
						util::Quatf n= util::Quatf::byRotationAxis(
									{next_v[0], next_v[1], next_v[2]}, next_v[3]);
						t.rotation= util::lerp(p, n, frac);
					} break;
					case ClipChannelType::scale: {
						t.scale= util::lerp(prev_v[0], next_v[0], frac);
					} break;
					default: fail("Unknown ClipChannelType");
				}
			}
		}
	}

	if (samples.empty()){
		throw resources::ResourceException(
			"animation::Clip %s has no data", getName().cStr());
	}
	else if (samples.front().size() != getArmature().getJoints().size()){
		throw resources::ResourceException(
				"animation::Clip %s contains wrong number of joints: %i != %i",
				getName().cStr(),
				samples.front().size(),
				getArmature().getJoints().size());
	}

	setResourceState(State::Loaded);
}

void Clip::unload(){
	samples.clear();
	setResourceState(State::Unloaded);
}

void Clip::loadBvh(){
	util::Str8 path= fileAttribute.get().whole();
	util::Str8 file_contents= global::File::readText(path);
	BvhData anim_data= parseBvhAnimation(file_contents);

	fps= 1.0/anim_data.frameTime;
	samples= inBindByLocalSamples(getArmature(),
				calcLocalPosesFromBvh(
					anim_data,
					jointNameToIdMap(getArmature())));
}

} // animation
} // clover
