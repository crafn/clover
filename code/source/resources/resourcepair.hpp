#ifndef CLOVER_RESOURCES_RESOURCE_PAIR_HPP
#define CLOVER_RESOURCES_RESOURCE_PAIR_HPP

#include "build.hpp"
#include "util/objectnodetraits.hpp"
#include "util/hash.hpp"
#include "util/string.hpp"

namespace clover {
namespace resources {

class Resource;

/// Unordered pair (for AttributeType::ResourcePair)
struct StrResourcePair {
	StrResourcePair(){}
	StrResourcePair(const util::Str8& a, const util::Str8& b);
	StrResourcePair(const Resource& a, const Resource& b);
	StrResourcePair(const Resource* a, const Resource* b);
	
	bool operator==(const StrResourcePair& o) const;
	
	util::Str8 first, second;
};

} // resources
namespace util {

template <>
class Hash32<resources::StrResourcePair> {
public:
	uint32 operator()(const resources::StrResourcePair& key) const {
		// Pair order doesn't matter
		return util::hash32(key.first) + util::hash32(key.second);
	}
};

template <>
struct ObjectNodeTraits<resources::StrResourcePair> {
	typedef resources::StrResourcePair Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_RESOURCES_RESOURCE_PAIR_HPP
