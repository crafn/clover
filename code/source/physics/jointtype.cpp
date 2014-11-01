#include "jointtype.hpp"
#include "global/exception.hpp"

namespace clover {
namespace physics {

util::Str8 RuntimeJointTypeTraits::enumString(JointType t){
	#define JOINT_TYPE(x) \
	if (t == JointType::x) \
		return #x;
	#include "jointtypes.def"
	#undef JOINT_TYPE

	throw global::Exception("Unexpected JointType: %i", (int)t);
}

JointType RuntimeJointTypeTraits::jointTypeByString(const util::Str8& name){
	#define JOINT_TYPE(x) \
	if (name == util::Str8(#x)) \
		return JointType::x;
	#include "jointtypes.def"
	#undef JOINT_TYPE

	throw global::Exception("Invalid joint type: %s", name.cStr());
}

} // clover
} // physics