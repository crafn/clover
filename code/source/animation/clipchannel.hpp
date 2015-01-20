#ifndef ANIMATION_CLIPCHANNEL_HPP
#define ANIMATION_CLIPCHANNEL_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "util/objectnodetraits.hpp"
#include "util/str8.hpp"
#include "util/vector.hpp"

namespace clover {
namespace animation {

enum class ClipChannelType {
	translation,
	rotation,
	scale
};

struct ClipChannel {
	ClipChannelType type;
	util::Str8 joint;
	util::DynArray<real32> times;
	util::DynArray<util::Vec3f> samples;
};

} // animation
namespace util {

template <>
struct ObjectNodeTraits<animation::ClipChannel> {
	typedef animation::ClipChannel Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // animation
} // util

#endif // ANIMATION_CLIPCHANNEL_HPP
