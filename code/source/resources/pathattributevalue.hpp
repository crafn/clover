#ifndef CLOVER_RESOURCES_PATHATTRIBUTEVALUE_HPP
#define CLOVER_RESOURCES_PATHATTRIBUTEVALUE_HPP

#include "build.hpp"
#include "util/string.hpp"
#include "resourcefilepath.hpp"
#include "util/filewatcher.hpp"

namespace clover {
namespace resources {

class PathAttributeValue : public ResourceFilePath {
public:
	PathAttributeValue(const util::Str8& attribute_value = "");
	PathAttributeValue(const char* c);
	PathAttributeValue(const util::Str8& resource_file_dir_from_root, const util::Str8& attribute_value);
	
	// None of the copy-stuff copies fileWatcher or callbacks
	PathAttributeValue(const PathAttributeValue& other);
	PathAttributeValue(PathAttributeValue&& other);
	
	virtual ~PathAttributeValue();
	
	PathAttributeValue& operator=(const PathAttributeValue& other);
	PathAttributeValue& operator=(PathAttributeValue&& other);
	
	
	/// Returns path relative from resource-file (that one which is put in the resource-file)
	const util::Str8& value() const;
	void setValue(const util::Str8& v);
	
	typedef util::FileWatcher::FileEvent FileEvent;
	typedef std::function<void (FileEvent)> OnFileChangeCallbackType;
	
	/// Called when file in which PathAttribute is pointing to is created, modified or destroyed
	void setOnFileChangeCallback(const OnFileChangeCallbackType& callback);
	
private:
	OnFileChangeCallbackType OnFileChangeCallback;
	
	void onFileChange(FileEvent);
	
	void updateWatcher();
	util::FileWatcher fileWatcher;
};

} // resources
} // clover

#endif // CLOVER_RESOURCES_PATHATTRIBUTEVALUE_HPP