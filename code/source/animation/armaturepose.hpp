#ifndef CLOVER_ANIMATION_ARMATUREPOSE_HPP
#define CLOVER_ANIMATION_ARMATUREPOSE_HPP

#include "build.hpp"
#include "jointpose.hpp"
#include "util/dyn_array.hpp"
#include "util/string.hpp"
#include "util/typestringtraits.hpp"

namespace clover {
namespace animation {
class Armature;

class ENGINE_API ArmaturePose {
public:
	using Pose= util::DynArray<JointPose>;

	/// Creates ArmaturePose by global pose around pose_origin
	static ArmaturePose byGlobalPose(const Armature& a, Pose global_pose);
	/// Creates ArmaturePose by local (not in-bind) pose
	static ArmaturePose byLocalPose(const Armature& a, Pose local_pose);

	ArmaturePose(); // Invalid value. Default constructor required by script
	bool isInvalid() const { return armature == nullptr; }

	ArmaturePose(const Armature& armature_, Pose local_in_bind_pose_);

	const Armature& getArmature() const;

	/// Transform of every joint is in coordinate system of bind pose joint
	/// @fixme	If 'this' is bind pose, getLocalInBindPose() returns
	///				bind-pose in _local_coordinates_, not in-bind coordinates!
	///				We should have another method: getLocalPose()
	const Pose& getLocalInBindPose() const { return localInBindPose; }

	/// Transform of every joint is in coordinate system of the Armature entity
	Pose getGlobalPose() const;

	/// Boost serialization

	template <typename Archive>
	void serialize(Archive& ar, uint32 version);

private:
	/// For serialization

	void setArmature(const util::Str8& name);
	const util::Str8& getArmatureName() const;

	Pose defaultPose() const;

	const Armature* armature= nullptr;
	Pose localInBindPose;
};

} // animation

animation::ArmaturePose::Pose lerp(	const animation::ArmaturePose::Pose& pose1,
									const animation::ArmaturePose::Pose& pose2,
									real64 f);

// Implementation

template <typename Archive>
void animation::ArmaturePose::serialize(Archive& ar, uint32 version) {
	if (Archive::is_saving::value) {
		auto name= getArmatureName();
		ar & name;
		ar & localInBindPose;
	} else {
		util::Str8 name;
		ar & name;
		ar & localInBindPose;
		setArmature(name);
	}
}

namespace util {

template <>
struct TypeStringTraits<animation::ArmaturePose> {
	static util::Str8 type(){ return "animation::ArmaturePose"; }
};

} // util
} // clover

#endif // CLOVER_ANIMATION_ARMATUREPOSE_HPP
