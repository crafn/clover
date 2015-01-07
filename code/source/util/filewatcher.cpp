#include "filewatcher.hpp"
#include "util/string.hpp"
#include "debug/print.hpp"
#include "util/map.hpp"

#if OS == OS_LINUX
	#include "filewatcher_impl_linux.hpp"
#elif OS == OS_WINDOWS
	#include "filewatcher_impl_windows.hpp"
#elif OS == OS_MACOSX
	#error "Todo: FileWatcher for mac";
#endif

namespace clover {
namespace util {

//
// FileWatcher
//

util::Mutex FileWatcher::mutex;
util::DynArray<FileWatcher::Launchable> FileWatcher::launchables;


void FileWatcher::globalInit(){
	FileWatcherImplType::globalInit();
}

void FileWatcher::globalShutdown(){
	FileWatcherImplType::globalShutdown();
}

FileWatcher::FileWatcher(){
	impl= std::move(std::shared_ptr<FileWatcherImpl>(new FileWatcherImplType()));
	impl->setOwner(*this);
}

FileWatcher::FileWatcher(FileWatcher&& other){
	util::LockGuard<util::Mutex> lock(mutex);
	
	impl= other.impl;
	other.impl.reset();
}

FileWatcher::~FileWatcher(){
	util::LockGuard<util::Mutex> lock(mutex);

	if (impl){
		impl->stopWatching();
		
		impl.reset();
	}
}

FileWatcher& FileWatcher::operator=(FileWatcher&& other){
	{
		util::LockGuard<util::Mutex> lock(mutex);

		impl= other.impl;
		other.impl.reset();
	}
	return *this;
}

void FileWatcher::setPath(const util::Str8& path){
	util::LockGuard<util::Mutex> lock(mutex);
	impl->setPath(path);
}

util::Str8 FileWatcher::getPath() const {
	util::LockGuard<util::Mutex> lock(mutex);
	return impl->getPath();
}

void FileWatcher::startWatching(){
	//print(debug::Ch::General, debug::Vb::Trivial, "startWatching begin");
	util::LockGuard<util::Mutex> lock(mutex);
	
	//print(debug::Ch::General, debug::Vb::Trivial, "startWatching(%s)", impl->getPath().cStr());
	impl->startWatching();
	//print(debug::Ch::General, debug::Vb::Trivial, "startWatching end");
}

void FileWatcher::stopWatching(){
	util::LockGuard<util::Mutex> lock(mutex);
	impl->stopWatching();
}


void FileWatcher::setOnChangeCallback(const OnChangeCallbackType& callback){
	util::LockGuard<util::Mutex> lock(mutex);
	impl->setOnChangeCallback(callback);
}

void FileWatcher::launchCallbacks(){
	decltype(launchables) copy;

	{
		// Impl modifies launchables, so must use the mutex of it
		FileWatcherImplType::lockFileThreadMutex();

		copy= launchables;
		launchables.clear();
		
		FileWatcherImplType::unlockFileThreadMutex();

	}
	
	// Avoid deadlock by calling callbacks when mutex is unlocked
	for (auto& m : copy){
		m.callback(m.event);
	}
}

void FileWatcher::addLaunchable(const Launchable& data){
	/// Called when impl has locked its file thread mutex!
	launchables.pushBack(data);
}

} // util
} // clover