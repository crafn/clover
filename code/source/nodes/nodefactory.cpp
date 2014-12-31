#include "nodefactory.hpp"
#include "resources/exception.hpp"
#include "script/module.hpp"
#include "util/string.hpp"
#include "util/traits.hpp"

#define NODEINSTANCE_HEADERS
#include "native_instances/native_instances.def"
#undef NODEINSTANCE_HEADERS

namespace clover {
namespace nodes {

NodeInstance* createNodeInstanceNativeLogic(const util::Str8& type_string)
{
	#define NODEINSTANCE(x) if (type_string == util::TypeStringTraits<x>::type()){ return new x; }
	#include "native_instances/native_instances.def"
	#undef NODEINSTANCE
	
	throw global::Exception("Invalid native NodeInstance name: %s", type_string.cStr());
}

CompNode* createCompNode(const util::Str8& type_string)
{
#define NODEINSTANCE(x) \
	if (type_string == util::TypeStringTraits<x>::type()) \
		return x::compNode();
#include "native_instances/native_instances.def"
#undef NODEINSTANCE

	throw global::Exception("Invalid native NodeInstance name: %s", type_string.cStr());
}

} // nodes
} // clover
