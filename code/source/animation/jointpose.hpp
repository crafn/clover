#ifndef CLOVER_ANIMATION_JOINTPOSE_HPP
#define CLOVER_ANIMATION_JOINTPOSE_HPP

#include "build.hpp"
#include "util/quaternion.hpp"
#include "util/transform.hpp"
#include "script/typestring.hpp"

namespace clover {
namespace animation {
	
using JointId= uint8;
const JointId JointIdNone= -1;
const SizeType maxJointCount= std::numeric_limits<JointId>::max() - 1;

struct JointPose {
	using Transform= util::SrtTransform<real32, util::Quatf, util::Vec3f>;

	JointPose(): jointId(JointIdNone){}

	/// Joint id in Armature
	JointId jointId;

	/// Defines joints coordinate space relative to parent (superjoint, bind pose or entity, depends on situation)
	/// e.g. ({1, 0, 0}, Quaternion::constructByRotationAxis({0, 0, 1}, util::pi/2), 2)
	/// implies that this joint is 1 unit away from the super along its x-axis,
	/// subjoints rotated 90 degrees counterclockwise and double sized
	Transform transform;

	template <typename Archive>
	void serialize(Archive& ar, uint32 version){
		ar & jointId;
		ar & transform;
	}
};

} // util

namespace util {

template <>
struct TypeStringTraits<animation::JointPose> {
	static util::Str8 type(){ return "::ArmatureJointPose"; }
};

} // util
} // clover

#endif // CLOVER_ANIMATION_JOINTPOSE_HPP