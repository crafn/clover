#ifndef CLOVER_ANIMATION_ASSIMP_UTIL_HPP
#define CLOVER_ANIMATION_ASSIMP_UTIL_HPP

#include "armaturepose.hpp"
#include "build.hpp"
#include "joint_def.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"
#include "util/unique_ptr.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace clover {
namespace animation {

struct AssimpData;

AssimpData parseAssimp(const util::Str8& path);

/// Local pose (not in-bind)
util::DynArray<ArmaturePose::Pose> localPosesFromAssimp(
		const AssimpData& ai,
		const util::HashMap<util::Str8, JointId>& joint_name_to_id);

util::DynArray<JointDef> jointDefsFromAssimp(const AssimpData& ai);

struct AssimpData {
	real64 fps= 30.0;
	SizeType frameCount;

private:
	friend
		AssimpData parseAssimp(const util::Str8&);
	friend
		util::DynArray<ArmaturePose::Pose>
		localPosesFromAssimp(const AssimpData& ai, const util::HashMap<util::Str8, JointId>&);
	friend
		util::DynArray<JointDef>
		jointDefsFromAssimp(const AssimpData& ai);

	util::UniquePtr<Assimp::Importer> importer;
	const aiScene* scene= nullptr;
	aiAnimation* animation= nullptr;
};

} // animation
} // clover

#endif // CLOVER_ANIMATION_ASSIMP_UTIL_HPP