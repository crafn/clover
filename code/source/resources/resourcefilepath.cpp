#include "global/env.hpp"
#include "resourcefilepath.hpp"
#include "cache.hpp"

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
		directoryPath == other.directoryPath)
		return true;
	return false;
}

const util::Str8& ResourceFilePath::relative() const {
	return relativePath;
}

void ResourceFilePath::setRelative(const util::Str8& r){
	relativePath= r;
}

util::Str8 ResourceFilePath::fromRoot() const {
	return (util::Str8(directoryPath + relativePath));
}

const util::Str8& ResourceFilePath::directoryFromRoot() const {
	return directoryPath;
}
	
util::Str8 ResourceFilePath::whole() const {
	return (util::Str8(global::g_env->resCache->getResourceRootPath() + fromRoot()));
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

} // resources
} // clover
