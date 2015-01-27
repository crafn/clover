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
	struct Key {
		real32 time;
		real32 value[4];
	};
	util::DynArray<Key> keys;
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
