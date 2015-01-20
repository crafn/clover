#include "cache.hpp"
#include "global/env.hpp"
#include "resourcefilepath.hpp"
#include "util/objectnode.hpp"

#include <boost/filesystem.hpp>

namespace clover {
namespace resources {

ResourceFilePath::ResourceFilePath(const util::Str8& dir, const util::Str8& rel):
	directoryPath(dir),
	relativePath(rel){
	
}

ResourceFilePath::~ResourceFilePath(){}

bool ResourceFilePath::operator==(const ResourceFilePath& other) const {
	if (relativePath == other.relativePath &&
		directoryPath == other.directoryPath &&
		ext == other.ext)
		return true;
	return false;
}

util::Str8 ResourceFilePath::relative() const {
	// Don't include `ext` in relative path to avoid saving with extension
	// Reason: "mymod" is mymod.dll on windows and mymod.so on linux
	return relativePath;
}

void ResourceFilePath::setRelative(const util::Str8& r){
	relativePath= r;
}

util::Str8 ResourceFilePath::fromRoot() const {
	return (util::Str8(directoryPath + relativePath + extPart()));
}

const util::Str8& ResourceFilePath::directoryFromRoot() const {
	return directoryPath;
}
	
util::Str8 ResourceFilePath::whole() const {
	return (util::Str8(global::g_env.resCache->getResourceRootPath() + fromRoot()));
}
	
bool ResourceFilePath::isValid() const {
	try {
		// Throws an exception if path is invalid
		boost::filesystem::path p= boost::filesystem::canonical(whole().cStr());
	}
	catch (...){
		return false;
	}
	return true;
}

void ResourceFilePath::setExt(const util::Str8& str)
{ ext= str; }

util::Str8 ResourceFilePath::extPart() const
{
	if (relativePath.empty() || ext.empty())
		return "";
	else
		return "." + ext;
}

} // resources
namespace util {

util::ObjectNode ObjectNodeTraits<resources::ResourceFilePath>::
serialized(const Value& value)
{
	util::ObjectNode ob;
	ob[0].setValue(value.directoryFromRoot());
	ob[1].setValue(value.relative());
	return (ob);
}

auto ObjectNodeTraits<resources::ResourceFilePath>::
deserialized(const util::ObjectNode& ob) -> Value
{
	Value v(ob.get(0).getValue<util::Str8>(), ob.get(1).getValue<util::Str8>());
	return (v);
}

} // util
} // clover
