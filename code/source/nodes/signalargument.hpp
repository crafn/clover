#ifndef CLOVER_NODES_SIGNALARGUMENT_HPP
#define CLOVER_NODES_SIGNALARGUMENT_HPP

#include "build.hpp"
#include "signaltype.hpp"
#include "util/objectnode.hpp"
#include "util/string.hpp"
#include "util/traits.hpp"

namespace clover {
namespace nodes {

/// Resource attribute type
struct SignalArgument {
	util::Str8 name;
	SignalType signalType;
	
	SignalArgument(const util::Str8& name_= "", SignalType s= SignalType::Real)
		: name(name_)
		, signalType(s){}
	
	bool operator==(const SignalArgument& other) const { return name == other.name && signalType == other.signalType; }
	bool operator!=(const SignalArgument& other) const { return !operator==(other); }
};

} // nodes
namespace util {

template <>
struct ObjectNodeTraits<nodes::SignalArgument> {
	typedef nodes::SignalArgument Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

template <>
struct TypeStringTraits<nodes::SignalArgument> {
	static util::Str8 type(){ return "::SignalArgument"; }
};

} // util
} // clover

#endif // CLOVER_NODES_SIGNALARGUMENT_HPP