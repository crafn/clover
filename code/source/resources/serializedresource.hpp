#ifndef CLOVER_RESOURCES_SERIALIZEDRESOURCE_HPP
#define CLOVER_RESOURCES_SERIALIZEDRESOURCE_HPP

#include "build.hpp"
#include "resourcefilepath.hpp"
#include "util/objectnode.hpp"

namespace clover {
namespace resources {
	
class Resource;

/// Contains all information for constructing a resource
class SerializedResource {
public:
	SerializedResource()= default;
	SerializedResource(const Resource& res);
	SerializedResource(const ResourceFilePath& path, const util::Str8& type_name, const util::ObjectNode& attribs);

	const ResourceFilePath& getPath() const { return path; }

	/// Section name in resource file
	const util::Str8& getTypeName() const { return typeName; }

	/// These exist in resource file
	const util::ObjectNode& getResourceAttributes() const { return attributes; }

private:
	ResourceFilePath path; // Path of file in which resource is defined
	util::Str8 typeName;
	util::ObjectNode attributes; // The json-object in resource file
};

} // resources
namespace util {

template <>
struct ObjectNodeTraits<resources::SerializedResource> {
	using Value= resources::SerializedResource;
	static util::ObjectNode serialized(const Value& value){
		util::ObjectNode ob;
		ob["path"].setValue(value.getPath());
		ob["typeName"].setValue(value.getTypeName());
		ob["attributes"].setValue(value.getResourceAttributes());
		return (ob);
	}
	
	static Value deserialized(const util::ObjectNode& ob){
		Value v(ob.get("path").getValue<resources::ResourceFilePath>(),
				ob.get("typeName").getValue<util::Str8>(),
				ob.get("attributes"));
		return (v);
	}
};

} // util
} // clover

#endif // CLOVER_RESOURCES_SERIALIZEDRESOURCE_HPP