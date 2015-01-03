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
	return global::g_env->resCache->getResource<Armature>(armatureAttribute.get());
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
	BvhData bvh_data= parseBvhAnimation(global::File::readText(path));
	fps= 1.0/bvh_data.frameTime;
	samples= calcLocalPosesFromBvh(bvh_data, jointNameToIdMap(getArmature()));

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
