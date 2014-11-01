#ifndef CLOVER_RESOURCES_BASESUBCACHE_HPP
#define CLOVER_RESOURCES_BASESUBCACHE_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace util {

class ObjectNode;

} // util
namespace resources {

class Resource;
class ResourceFilePath;
class SerializedResource;

/// @todo util::ObjectNode, Resource type and ResourceFilePath should be wrapped together to form a SerializedResource

class BaseSubCache {
public:

	virtual ~BaseSubCache(){}

	// Implement these when inheriting
	/*
		const ResourceType& getResource(const IdentifierValue& key)= 0; // Returns always usable resource
		const ResourceType* findResource(const IdentifierValue& key)= 0; // Returns null-pointer if no matching resource is found
	*/

	virtual const Resource& getResource(const util::ObjectNode& res_id)= 0;

	virtual const Resource& getErrorResource()= 0;

	/// @return util::ObjectNode field name which this class can parse
	virtual util::Str8 getResourceTypeName() const = 0;

	/// Same as parseSection but only for single Resource
	virtual void parseResource(const SerializedResource&) = 0;

	/// Reads util::ObjectNode-structure and creates/updates corresponding resource-objects (not necessarily load them yet)
	virtual void parseResourceSection(const util::ObjectNode& root, const ResourceFilePath& resource_file_path)= 0;

	/// Generates and returns json-value so that this->parse(generateSection(path), path) == *this
	virtual util::ObjectNode generateSection(const ResourceFilePath& path) = 0;

	/// Loads stuff like mipmaps for textures
	virtual void preLoad()= 0;

	/// Called once per frame
	virtual void update()= 0;

	virtual const util::DynArray<Resource*>& getGenericResources() const = 0;
private:
};

} // resources
} // clover

#endif // CLOVER_RESOURCES_BASESUBCACHE_HPP