#include "animation/clipchannel.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace animation {


} // animation
namespace util {

util::ObjectNode ObjectNodeTraits<animation::ClipChannel>::
serialized(const Value& value)
{
	return util::ObjectNode{};
}

auto ObjectNodeTraits<animation::ClipChannel>::
deserialized(const util::ObjectNode& ob_node) -> Value
{
	return Value();
}

} // util
} // clover
