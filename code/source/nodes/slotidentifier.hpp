#ifndef CLOVER_NODES_SLOTIDENTIFIER_HPP
#define CLOVER_NODES_SLOTIDENTIFIER_HPP

#include "build.hpp"
#include "signaltypetraits.hpp"
#include "util/hash.hpp"
#include "util/string.hpp"

namespace clover {
namespace nodes {

/// Identifies a slot nodewise, but separately for input and outputs
/// (= it's possible that certain input and output slot has the same identifier in the same node)
/// @todo Add input/output bool to identifier
struct SlotIdentifier {
	util::Str8 name;
	util::Str8 groupName;
	SignalType signalType;
	bool input;
	
	bool operator==(const SlotIdentifier& other) const { return name == other.name && signalType == other.signalType && input == other.input; }
	bool operator!=(const SlotIdentifier& other) const { return !(*this == other); }

	util::Str8 getString() const { return name + ", Group: " + groupName + util::Str8::format(", Input: %i", input) + ", " + RuntimeSignalTypeTraits::enumString(signalType); }
	static util::Str8 typeString(){ return "SlotIdentifier"; }

	template <typename Archive>
	void serialize(Archive& ar, const uint32 version){
		ar & name;
		ar & groupName;
		ar & signalType;
		ar & input;
	}
};

} // nodes
namespace util {

template <>
class Hash32<nodes::SlotIdentifier> {
public:
	uint32 operator()(const nodes::SlotIdentifier& id) const {
		return util::hash32(id.name) + util::hash32(id.groupName) + (int32)id.signalType + (id.input ? 0 : 9999);
	}
};

template <>
struct TypeStringTraits<nodes::SlotIdentifier> {
	static util::Str8 type(){ return "::SlotIdentifier"; }
};

} // util
} // clover

#endif // CLOVER_NODES_SLOTIDENTIFIER_HPP