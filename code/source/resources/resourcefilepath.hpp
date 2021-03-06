#ifndef CLOVER_RESOURCES_RESOURCEFILEPATH_HPP
#define CLOVER_RESOURCES_RESOURCEFILEPATH_HPP

#include "build.hpp"
#include "util/hash.hpp"
#include "util/objectnodetraits.hpp"
#include "util/string.hpp"

namespace clover {
namespace resources {

class ResourceFilePath {
public:
	/// dir is from resource root
	ResourceFilePath(const util::Str8& dir= "", const util::Str8& rel= "");
	virtual ~ResourceFilePath();
	
	bool operator==(const ResourceFilePath& other) const;
	
	/// Returns path relative from resource-file directory
	util::Str8 relative() const;
	void setRelative(const util::Str8& r);
	
	/// Returns path from resource root
	util::Str8 fromRoot() const;
	
			
	/// Returns directory of resource file from resource root
	const util::Str8& directoryFromRoot() const;
	
	/// Whole path which can be used to open files
	util::Str8 whole() const;
	
	bool isValid() const;

	void setExt(const util::Str8& str);
	
	template <typename Archive>
	void serialize(Archive& ar, uint32 ver){
		ar & directoryPath;
		ar & relativePath;
	}
	
private:
	util::Str8 extPart() const;

	util::Str8 directoryPath, relativePath;
	util::Str8 ext;
};

} // resources
namespace util {

template <>
class Hash32<resources::ResourceFilePath> {
public:
	uint32 operator()(const resources::ResourceFilePath& path) const {
		return util::hash32(path.fromRoot());
	}
};

template <>
struct ObjectNodeTraits<resources::ResourceFilePath> {
	using Value= resources::ResourceFilePath;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob);
};

} // util
} // clover

#endif // CLOVER_RESOURCES_RESOURCEFILEPATH_HPP
