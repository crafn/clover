#include "signalargument.hpp"
#include "nodes/signaltypetraits.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace util {

util::ObjectNode ObjectNodeTraits<nodes::SignalArgument>::serialized(const Value& value){
	util::ObjectNode ret;

	ret["Name"].setValue<util::Str8>(value.name);
	ret["SignalType"].setValue<util::Str8>(nodes::RuntimeSignalTypeTraits::enumString(value.signalType));

	return (ret);
}

auto ObjectNodeTraits<nodes::SignalArgument>::deserialized(const util::ObjectNode& ob_node) -> Value {
	Value ret;
	ret.name= ob_node.get("Name").getValue<util::Str8>();
	ret.signalType= nodes::RuntimeSignalTypeTraits::signalTypeFromEnumString(ob_node.get("SignalType").getValue<util::Str8>());

	return (ret);
}

} // util
} // clover
