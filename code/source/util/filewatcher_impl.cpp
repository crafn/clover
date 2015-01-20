#include "debug/print.hpp"
#include "filewatcher_impl.hpp"
#include "util/ensure.hpp"

#include <boost/filesystem.hpp>

namespace clover {
namespace util {

FileWatcherImpl::FileWatcherImpl():
	owner(0){}

FileWatcherImpl::~FileWatcherImpl(){}

void FileWatcherImpl::setOwner(FileWatcher& o){ owner= &o; }

void FileWatcherImpl::setPath(const util::Str8& _path){
	try {
		boost::filesystem::path p= boost::filesystem::canonical(_path.cStr());
		path= p.string();
	}
	catch(...){
		debug::print("FileWatcher got invalid path: %s", _path.cStr());
		/// Invalid path
		path= _path;
	}
	
}

const util::Str8& FileWatcherImpl::getPath() const { return path; }

void FileWatcherImpl::setOnChangeCallback(const FileWatcher::OnChangeCallbackType& callback){ OnChange= callback; }

// Called in thread
void FileWatcherImpl::addLaunchable(FileWatcher::FileEvent e){
	ensure(owner);
	FileWatcher::Launchable data;
	data.callback= OnChange;
	data.event= e;
	owner->addLaunchable(data);
}

} // util
} // clover
