#include "resourcepair.hpp"
#include "resource.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace resources {

StrResourcePair::StrResourcePair(const util::Str8& a, const util::Str8& b):
	first(a), second(b){
		
}

StrResourcePair::StrResourcePair(const Resource& m1, const Resource& m2):
	StrResourcePair(&m1, &m2){
}

StrResourcePair::StrResourcePair(const Resource* m1, const Resource* m2){
	if (m1)
		first= m1->getIdentifierAsString();
	if (m2)
		second= m2->getIdentifierAsString();
}
	
bool StrResourcePair::operator==(const StrResourcePair& o) const {
	return (first == o.first && second == o.second) || (first == o.second && second == o.first);
}

} // resources
namespace util {

util::ObjectNode ObjectNodeTraits<resources::StrResourcePair>::
serialized(const Value& value)
{
	util::ObjectNode ret;
	ret.append(value.first);
	ret.append(value.second);
	return (ret);
}

auto ObjectNodeTraits<resources::StrResourcePair>::
deserialized(const util::ObjectNode& ob_node) -> Value
{
	return Value(ob_node.get(0).getValue<util::Str8>(), ob_node.get(1).getValue<util::Str8>());
}

} // util
} // clover
