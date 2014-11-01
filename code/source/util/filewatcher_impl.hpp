#ifndef CLOVER_UTIL_FILEWATCHER_IMPL_HPP
#define CLOVER_UTIL_FILEWATCHER_IMPL_HPP

#include "build.hpp"
#include "filewatcher.hpp"
#include "util/string.hpp"

namespace clover {
namespace util {

// Declare in implementations
//typedef FileWatcherImpl FileWatcherImplType;
class FileWatcherImpl {
public:
	// Implement in implementations
	//static void globalInit();
	//static void globalShutdown();

	FileWatcherImpl();
	FileWatcherImpl(const FileWatcherImpl&)= delete;
	FileWatcherImpl(FileWatcherImpl&&)= delete;
	virtual ~FileWatcherImpl();
	
	void setOwner(FileWatcher& o);

	void setPath(const util::Str8& path);
	const util::Str8& getPath() const;
	virtual void startWatching()= 0;
	virtual void stopWatching()= 0;
	
	/// These need to be defined in inherited types
	/// Locks mutex which protects addLaunchable-call
	//static void lockFileThreadMutex();
	//static void unlockFileThreadMutex();

	void setOnChangeCallback(const FileWatcher::OnChangeCallbackType& callback);

	// Called in thread
	void addLaunchable(FileWatcher::FileEvent e);

protected:
	FileWatcher::OnChangeCallbackType OnChange;
	FileWatcher* owner;

	util::Str8 path;
};

} // util
} // clover

#endif // CLOVER_UTIL_FILEWATCHER_IMPL_HPP