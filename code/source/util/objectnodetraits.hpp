#ifndef UTIL_OBJECTNODETRAITS_HPP
#define UTIL_OBJECTNODETRAITS_HPP

namespace clover {
namespace util {

/// Enable is used to switch traits on/off depending on T
template <typename T, typename Enable= void>
struct ObjectNodeTraits;

class ObjectNode;

} // util
} // clover

#endif // UTIL_OBJECTNODE_TRAIT_HPP
