#ifndef CLOVER_UTIL_FILEWATCHER_HPP
#define CLOVER_UTIL_FILEWATCHER_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"

/// @todo Replace with util::Thread
#include <boost/thread.hpp>
#include <functional>
#include <memory>

namespace clover {
namespace util {

class FileWatcherImpl;

/// Notifies when file (or files) change
class FileWatcher {
public:

	enum class FileEvent : uint32 {
		Create,
		Delete,
		Modify
	};

	typedef std::function<void (FileEvent)> OnChangeCallbackType;

	// Call before any FileWatchers is created
	static void globalInit();
	static void globalShutdown();

	FileWatcher();
	FileWatcher(FileWatcher&&);
	virtual ~FileWatcher();

	FileWatcher& operator=(FileWatcher&&);

	/// Non-blocking. OnChangeCallback is called when something happens to the file
	void setPath(const util::Str8& path);
	util::Str8 getPath() const;
	void startWatching();
	void stopWatching();

	void setOnChangeCallback(const OnChangeCallbackType& callback);

	/// Call only from main thread
	static void launchCallbacks();

protected:
	friend class FileWatcherImpl;

	struct Launchable {
		OnChangeCallbackType callback;
		FileEvent event;
	};

	/// Called when impl has locked its file thread mutex!
	void addLaunchable(const Launchable&);

private:
	std::shared_ptr<FileWatcherImpl> impl;

	static util::DynArray<Launchable> launchables;
	static boost::mutex mutex;
};

} // util
} // clover

#endif // CLOVER_UTIL_FILEWATCHER_HPP