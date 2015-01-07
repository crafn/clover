#include "build.hpp"
#if OS == OS_LINUX
#include "filewatcher_impl_linux.hpp"
#include "debug/print.hpp"
#include "util/ensure.hpp"
#include "util/thread.hpp"

#include <boost/filesystem.hpp>

namespace clover {
namespace util {

int FileWatcherLinuxImpl::fd= -1;
int FileWatcherLinuxImpl::watchCount= 0;
std::mutex FileWatcherLinuxImpl::implMapMutex;
Map<int, util::DynArray<FileWatcherLinuxImpl::ImplWrap>> FileWatcherLinuxImpl::implMap;
std::thread FileWatcherLinuxImpl::fileWatcherThread;

void FileWatcherLinuxImpl::globalInit(){
	fd= inotify_init();

	ensure(watchCount == 0);
	ensure_msg(fd >= 0, "inotify_init() failed");
}

void FileWatcherLinuxImpl::globalShutdown(){
	ensure_msg(watchCount == 0, "watchCount == %i != 0", watchCount);
	ensure(fd >= 0);
	close(fd);
	fd= -1;
}

FileWatcherLinuxImpl::FileWatcherLinuxImpl():
	wd(-1){}

FileWatcherLinuxImpl::~FileWatcherLinuxImpl(){
	ensure_msg(wd == -1, "wd: %i", wd);
}

void FileWatcherLinuxImpl::startWatching(){
	if (wd >= 0)
		removeWatch();

	addWatch();
}

void FileWatcherLinuxImpl::stopWatching(){
	if (wd >= 0){
		removeWatch();
	}
}

void FileWatcherLinuxImpl::lockFileThreadMutex(){
	implMapMutex.lock();
}

void FileWatcherLinuxImpl::unlockFileThreadMutex(){
	implMapMutex.unlock();
}

// Called from main thread and from wait thread
void FileWatcherLinuxImpl::addWatch(){
	
	ensure(fd >= 0);
	ensure(wd == -1);

	boost::filesystem::path boost_path(path.cStr());
	util::Str8 watch_path= boost_path.parent_path().string().c_str();
	
	{
		LockGuard<Mutex> lock(implMapMutex);
		
		for (auto it= implMap.begin(); it != implMap.end(); ++it){
			if (!it->second.empty() && it->second.front().dirPath == watch_path){
				wd= it->first;
				break;
			}
		}
		
		if (wd == -1){
			// Create inotify watch for directory
			wd= inotify_add_watch(fd, watch_path.cStr(), IN_ALL_EVENTS);
				
			if (wd == -1)
				return; // Invalid path
				
			//print(debug::Ch::General, debug::Vb::Moderate, "FileWatcher: watching directory %s", watch_path.cStr());
		}

		implMap[wd].pushBack({boost_path.filename().c_str(), watch_path, this});
	}
	{
		LockGuard<Mutex> lock(watchMutex);
		++watchCount;
		
		if (watchCount == 1){
			fileWatcherThread= std::thread(waitForChange);
			setThreadPriority(fileWatcherThread, ThreadPriority::Idle);
		}
	}
}

// Called from main thread and from wait thread
void FileWatcherLinuxImpl::removeWatch(){
	
	if (wd >= 0){
		watchMutex.lock();
		--watchCount;
		ensure(watchCount >= 0);
		watchMutex.unlock();
		
		{
			LockGuard<Mutex> lock(implMapMutex);

			auto it= implMap.find(wd);
			ensure_msg(it != implMap.end(), "Wd not found: %i, %s, watchCount: %i", wd, path.cStr(), watchCount);
			if (it->second.size() == 1){
				implMap.erase(it);
				inotify_rm_watch(fd, wd);
			}
			else {
				bool found= false;
				for (auto it2= it->second.begin(); it2 != it->second.end(); ++it2){
					if (it2->impl == this){
						it->second.erase(it2);
						found= true;
						break;
					}
				}
				
				ensure(found);
			
			}
			//print(debug::Ch::General, debug::Vb::Moderate, "FileWatcher: removíng %s", path.cStr());
		}

		wd= -1;
		
		if (watchCount == 0){
			fileWatcherThread.detach();
		}

		// Erasing from implMap is done in thread
	}
}

/// Entry for fileWatcherThread
void FileWatcherLinuxImpl::waitForChange(){

	constexpr uint32 buflen= sizeof(inotify_event)*50;
	char buf[buflen];

	ssize_t ret= 0;

	while(fd >= 0 && watchCount > 0){

		ret= read(fd, buf, buflen);
		if (ret < 0){
			if (errno == EINTR){
				continue;
			}
			else {
				print(debug::Ch::General, debug::Vb::Trivial, "FileWatcher thread read error: %li, %i, %s", ret, errno, strerror(errno));
				break;
			}
		}
		
		int32 i= 0;

		while (i < ret && fd >= 0 && watchCount > 0){

			inotify_event* event= reinterpret_cast<inotify_event*>(&buf[i]);

			i += sizeof(inotify_event) + event->len;

			if (event->mask & IN_IGNORED){
				//print(debug::Ch::General, debug::Vb::Trivial, "Filewatch ignored %i", event->wd);
				continue;
			}


			FileWatcher::FileEvent e;
					
			if (event->mask & IN_CREATE){
				e= FileWatcher::FileEvent::Create;
				//print(debug::Ch::General, debug::Vb::Trivial, "File create %i", event->wd);
			}
			else if ((event->mask & IN_DELETE) || (event->mask & IN_MOVE)){
				e= FileWatcher::FileEvent::Delete;
				//print(debug::Ch::General, debug::Vb::Trivial, "File delete/move %i", event->wd);
			}
			else if (event->mask & IN_CLOSE_WRITE){
				e= FileWatcher::FileEvent::Modify;
				//print(debug::Ch::General, debug::Vb::Trivial, "File close_write %i", event->wd);
			}
			else if (event->mask & IN_MODIFY){

				//print(debug::Ch::General, debug::Vb::Trivial, "File write %i", event->wd);
				continue;
			}
			else if (event->mask & IN_CLOSE){
				//print(debug::Ch::General, debug::Vb::Trivial, "File close %i", event->wd);
				continue;
			}
			else if (event->mask & IN_OPEN){
				//print(debug::Ch::General, debug::Vb::Trivial, "File open %i", event->wd);

				continue;
			}
			else if (event->mask & IN_ACCESS){
				//print(debug::Ch::General, debug::Vb::Trivial, "File access %i", event->wd);
				continue;
			}
			else if (event->mask & IN_ATTRIB){
				continue;
			}
			else if (event->mask & IN_MOVE){
				//print(debug::Ch::General, debug::Vb::Trivial, "File move %i", event->wd);
				continue;
			}
			else {
				release_ensure_msg(0, "Impossible mask: %i", event->mask);
			}

			{
				LockGuard<Mutex> lock(implMapMutex);
					
				auto it= implMap.find(event->wd);
				ensure(it != implMap.end());
				auto impl_ptrs= it->second;
				if (!impl_ptrs.empty()){
					//print(debug::Ch::General, debug::Vb::Trivial, "FileWatcher: global::File event for %s/%s", impl_ptrs.front().dirPath.cStr(), event->name);
				}
				else {
					//print(debug::Ch::General, debug::Vb::Trivial, "EMPTY global::File event %s", event->name);
				}
				
				for (auto& m : impl_ptrs){
					if (m.fileName == util::Str8(event->name)){
						m.impl->addLaunchable(e);
					}
				}
			}
		}
	}
	//print(debug::Ch::General, debug::Vb::Trivial, "Filewatch thread quit");
}

} // util
} // clover

#endif // OS_LINUX
