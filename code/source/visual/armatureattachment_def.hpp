#ifndef CLOVER_VISUAL_ARMATUREATTACHMENT_DEF_HPP
#define CLOVER_VISUAL_ARMATUREATTACHMENT_DEF_HPP

#include "build.hpp"
#include "util/objectnode.hpp"
#include "util/optional.hpp"
#include "util/transform.hpp"

namespace clover {
namespace visual {
	
struct ArmatureAttachmentDef {
	using Transform= util::SrtTransform<real32, util::Quatf, util::Vec3f>;
	
	util::Str8 entityName;
	util::Str8 jointName;
	util::Optional<Transform> offset;
};
	
} // visual

namespace util {

template <>
struct ObjectNodeTraits<visual::ArmatureAttachmentDef>{
	using Value= visual::ArmatureAttachmentDef;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob);
};

} // util
} // clover

#endif // CLOVER_VISUAL_ARMATUREATTACHMENT_DEF_HPP
