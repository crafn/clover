#ifndef CLOVER_UTIL_FILEWATCHER_IMPL_WINDOWS_HPP
#define CLOVER_UTIL_FILEWATCHER_IMPL_WINDOWS_HPP

#include "build.hpp"

#if OS == OS_WINDOWS

#include "filewatcher_impl.hpp"
#include "util/map.hpp"
#include "util/dyn_array.hpp"

#include <thread>
#include <windows.h>

namespace clover {
namespace util {

// Works only for resource files for now
class FileWatcherWindowsImpl : public FileWatcherImpl {
public:
	static void globalInit();
	static void globalShutdown();

	FileWatcherWindowsImpl();
	virtual ~FileWatcherWindowsImpl();


	virtual void startWatching();
	virtual void stopWatching();

	static void lockFileThreadMutex();
	static void unlockFileThreadMutex();

private:
	static void directoryChangeDetectionLoop(const util::Str8& path);

	static std::thread changeDetectionThread;
	static int32 watchCount;
	static Map<util::Str8, util::DynArray<FileWatcherWindowsImpl*>> implMap;
	static boost::mutex implMapMutex;
	static HANDLE dirHandle;

	void addWatch();
	void removeWatch();

	bool watching;
	HANDLE handle;
};

typedef FileWatcherWindowsImpl FileWatcherImplType;

} // util
} // clover

#endif // OS_WINDOWS
#endif // UTIL_FILEWATCHER_IMPL_WINDOWS_HPP