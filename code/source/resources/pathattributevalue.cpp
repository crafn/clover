#include "pathattributevalue.hpp"
#include "util/ensure.hpp"
#include "debug/debugprint.hpp"

namespace clover {
namespace resources {

PathAttributeValue::PathAttributeValue(const util::Str8& attribute_value):
	ResourceFilePath("", attribute_value){
		
}
	
PathAttributeValue::PathAttributeValue(const char* c):
	PathAttributeValue(util::Str8(c)){}

PathAttributeValue::PathAttributeValue(const util::Str8& resource_file_dir_from_root, const util::Str8& attribute_value):
	ResourceFilePath(resource_file_dir_from_root, attribute_value){
		
}

PathAttributeValue::PathAttributeValue(const PathAttributeValue& other):
	ResourceFilePath(other){
	
	fileWatcher.setOnChangeCallback([&] (FileEvent e) { onFileChange(e); });
	updateWatcher();
}

PathAttributeValue::PathAttributeValue(PathAttributeValue&& other):
	ResourceFilePath(std::move(other)){
		
	fileWatcher.setOnChangeCallback([&] (FileEvent e) { onFileChange(e); });

	updateWatcher();
}

PathAttributeValue::~PathAttributeValue(){}

PathAttributeValue& PathAttributeValue::operator=(const PathAttributeValue& other){
	ResourceFilePath::operator=(other);

	updateWatcher();
	
	return *this;
}
PathAttributeValue& PathAttributeValue::operator=(PathAttributeValue&& other){
	ResourceFilePath::operator=(other);
	
	updateWatcher();
	
	return *this;
}

const util::Str8& PathAttributeValue::value() const { return relative(); }

void PathAttributeValue::setValue(const util::Str8& v){
	setRelative(v);
}

void PathAttributeValue::setOnFileChangeCallback(const OnFileChangeCallbackType& callback){
	OnFileChangeCallback= callback;
}

void PathAttributeValue::onFileChange(FileEvent e){
	print(debug::Ch::Resources, debug::Vb::Trivial, "File changed: %s, event: %i", fromRoot().cStr(), (uint32)e);
	
	if (OnFileChangeCallback)
		OnFileChangeCallback(e);

}

void PathAttributeValue::updateWatcher(){
	
	if (value().empty()){
		fileWatcher.stopWatching();
	}
	else {
		fileWatcher.setPath(whole());
		fileWatcher.startWatching();
	}
}

} // resources
} // clover