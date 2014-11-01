#ifndef CLOVER_UTIL_FILEWATCHER_IMPL_LINUX_HPP
#define CLOVER_UTIL_FILEWATCHER_IMPL_LINUX_HPP

#include "build.hpp"

#if OS == OS_LINUX

#include "filewatcher_impl.hpp"
#include "util/map.hpp"
#include "util/mutex.hpp"

#include <sys/inotify.h>

namespace clover {
namespace util {

class FileWatcherLinuxImpl : public FileWatcherImpl {
public:
	static void globalInit();
	static void globalShutdown();

	FileWatcherLinuxImpl();
	virtual ~FileWatcherLinuxImpl();

	virtual void startWatching();
	virtual void stopWatching();

	static void lockFileThreadMutex();
	static void unlockFileThreadMutex();
	
private:
	static int fd;
	static int watchCount;
	static Mutex implMapMutex;
	
	struct ImplWrap {
		util::Str8 fileName;
		util::Str8 dirPath; // Same for every ImplWrap in util::DynArray, but cleaner to implement this way
		FileWatcherLinuxImpl* impl;
	};
	static Map<int, util::DynArray<ImplWrap>> implMap;
	static std::thread fileWatcherThread;

	int wd;

	Mutex watchMutex;

	// Called from main thread and from wait thread
	void addWatch();
	// Called from main thread and from wait thread
	void removeWatch();

	/// Entry for fileWatcherThread
	static void waitForChange();
};

typedef FileWatcherLinuxImpl FileWatcherImplType;

} // util
} // clover

#endif // OS_LINUX

#endif // CLOVER_UTIL_FILEWATCHER_IMPL_LINUX_HPP