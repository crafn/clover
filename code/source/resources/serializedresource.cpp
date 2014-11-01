#include "serializedresource.hpp"
#include "resource.hpp"
#include "resourcefilepath.hpp"
#include "util/ensure.hpp"

namespace clover {
namespace resources {

SerializedResource::SerializedResource(const Resource& res)
		: path(res.getResourceFilePath())
		, typeName(res.getResourceTypeName()){
	auto& res_attributes= res.getResourceAttributes();

	for (const auto& attrib_pair : res_attributes){
		const auto& res_attrib= attrib_pair.second;

		try {
			attributes[res_attrib->getKey()]= res_attrib->serialized();
			ensure(!attributes.get(res_attrib->getKey()).isNull());
		}
		catch (const ResourceException& e){
			print(debug::Ch::Resources, debug::Vb::Critical,
				"(%s) Attribute %s serializing failed for %s", typeName.cStr(), res_attrib->getKey().cStr(), res.getIdentifierAsString().cStr());
		}
	}
}

SerializedResource::SerializedResource(const ResourceFilePath& path_, const util::Str8& type_name, const util::ObjectNode& attribs)
	: path(path_)
	, typeName(type_name)
	, attributes(attribs){}

} // resources
} // clover