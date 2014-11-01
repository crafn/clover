#ifndef CLOVER_RESOURCES_ATTRIBUTEPARSEINFO_HPP
#define CLOVER_RESOURCES_ATTRIBUTEPARSEINFO_HPP

#include "build.hpp"

namespace clover {
namespace util {

class ObjectNode;

} // util
namespace resources {

struct AttributeParseInfo {
	AttributeParseInfo(const util::ObjectNode& val, const util::Str8& dir):
		object(val),
		resourceDirectory(dir){}
	
	const util::ObjectNode& object; // Json value of the attribute
	const util::Str8& resourceDirectory; // Path to directory of the .res file in which the attribute is
};

} // resources
} // clover

#endif // CLOVER_RESOURCES_ATTRIBUTEPARSEINFO_HPP