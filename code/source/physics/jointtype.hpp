#ifndef CLOVER_PHYSICS_JOINTTYPE_HPP
#define CLOVER_PHYSICS_JOINTTYPE_HPP

#include "build.hpp"
#include "util/str8.hpp"

namespace clover {
namespace physics {

enum class JointType {
	#define JOINT_TYPE(x) x,
	#include "jointtypes.def"
	#undef JOINT_TYPE
	Last
};

struct RuntimeJointTypeTraits {
	static util::Str8 enumString(JointType t);
	static JointType jointTypeByString(const util::Str8& name);
};

} // physics
} // clover

#endif // CLOVER_PHYSICS_JOINTTYPE_HPP
