#include "build.hpp"
#if OS == OS_WINDOWS
#include "filewatcher_impl_windows.hpp"
#include "global/env.hpp"
#include "util/string.hpp"
#include "util/thread.hpp"
#include "resources/cache.hpp"

#include <boost/filesystem.hpp>

namespace clover {
namespace util {

void cancelIo(HANDLE h){
	typedef BOOL (WINAPI *Func)(HANDLE, LPOVERLAPPED);
	Func f= (Func)(GetProcAddress(LoadLibrary("kernel32.dll"), "CancelIoEx"));
	f(h, NULL);
}

std::thread FileWatcherWindowsImpl::changeDetectionThread;
int32 FileWatcherWindowsImpl::watchCount= 0;
Map<util::Str8, util::DynArray<FileWatcherWindowsImpl*>> FileWatcherWindowsImpl::implMap;
util::Mutex FileWatcherWindowsImpl::implMapMutex;
HANDLE FileWatcherWindowsImpl::dirHandle;

void FileWatcherWindowsImpl::globalInit(){

}

void FileWatcherWindowsImpl::globalShutdown(){
	ensure(watchCount == 0);
	// Quit thread
	cancelIo(dirHandle);
	changeDetectionThread.join();
}

FileWatcherWindowsImpl::FileWatcherWindowsImpl():
	watching(false), handle(0){
}

FileWatcherWindowsImpl::~FileWatcherWindowsImpl(){
}

void FileWatcherWindowsImpl::startWatching(){
	ensure(!watching);
	addWatch();
}

void FileWatcherWindowsImpl::stopWatching(){
	if (watching)
		removeWatch();
}


void FileWatcherWindowsImpl::lockFileThreadMutex(){
	implMapMutex.lock();
}

void FileWatcherWindowsImpl::unlockFileThreadMutex(){
	implMapMutex.unlock();
}

void FileWatcherWindowsImpl::addWatch(){
	if (!watching){
		if (watchCount == 0){
			changeDetectionThread= std::thread(directoryChangeDetectionLoop, global::g_env.resCache->getResourceRootPath());
			setThreadPriority(changeDetectionThread, ThreadPriority::Idle);
		}

		watching= true;

		++watchCount;

		handle= CreateFile( TEXT(path.cStr()),		// filename
							GENERIC_READ,			// read
							0,						// do not share
							NULL,					// default security
							OPEN_EXISTING,			// create new file only
							FILE_ATTRIBUTE_NORMAL,
							NULL);					// no attr. template

		{
			util::LockGuard<util::Mutex> lock(implMapMutex);

			implMap[path].pushBack(this);
			//print(debug::Ch::General, debug::Vb::Trivial, "Watch handle added: %s, %i, %i", path.cStr(), handle, implMap[path].size());
		}

		CloseHandle(handle);
	}
	//print(debug::Ch::General, debug::Vb::Trivial, "addWatch end");
}

void FileWatcherWindowsImpl::removeWatch(){
	ensure(watching);

	//print(debug::Ch::General, debug::Vb::Trivial, "Watch handle removed: %s, %i", path.cStr(), handle, implMap[path].size());

	{
		util::LockGuard<util::Mutex>lock(implMapMutex);

		auto it= implMap.find(path);
		ensure_msg(it != implMap.end(), "Watch not found: %s, watchCount: %i",	path.cStr(), watchCount);
		if (it->second.size() == 1){
			implMap.erase(it);

		}
		else {
			auto it2= it->second.find(this);
			ensure(it2 != it->second.end());
			it->second.erase(it2);
		}

	}

	/// @todo Shutdown thread somehow
	if (watchCount == 0)
		changeDetectionThread.detach();

	--watchCount;
	watching= false;
	handle= 0;
}


void FileWatcherWindowsImpl::directoryChangeDetectionLoop(const util::Str8& dir_path){

	auto dir_wpath= std::unique_ptr<WCHAR>(new WCHAR[dir_path.length()+1]);

	int32 convert_ret=	MultiByteToWideChar(
				CP_ACP,
				0,
				dir_path.cStr(),
				dir_path.length(),
				dir_wpath.get(),
				dir_path.length());

	dir_wpath.get()[dir_path.length()]= 0;

	if (convert_ret < 0){
		print(debug::Ch::General, debug::Vb::Critical, "FileWatcherWindowsImpl::directoryChangeDetectionLoop(..): path conversion: %i", convert_ret);
	}

	HANDLE dir_handle;
	//_tsplitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);

	// Watch the directory for file creation and deletion.
	dir_handle= CreateFileW( dir_wpath.get(),				// path to the directory
							FILE_LIST_DIRECTORY,	// needed by ReadDirectoryChanges
							FILE_SHARE_READ,		//
							NULL,					// default security
							OPEN_EXISTING,			// create new file only
							FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL,
							NULL);					// no attr. template

	if (dir_handle == INVALID_HANDLE_VALUE){
		print(debug::Ch::General, debug::Vb::Critical, "ERROR: CreateFile function failed, %lu path: %s", GetLastError(), dir_path.cStr());
		ExitProcess(GetLastError());
	}
	if (dir_handle == NULL){
		print(debug::Ch::General, debug::Vb::Critical, "ERROR: Unexpected NULL from CreateFile.");
		ExitProcess(GetLastError());
	}

	dirHandle= dir_handle;
	dir_wpath.reset();

	DWORD buf_size= 1024;
	char buf[buf_size];

	while (1){
		// Wait for notification.
		//print(debug::Ch::General, debug::Vb::Trivial, "FileWatcherWindowsImpl::directoryChangeDetectionLoop(..): Waiting for notification...");
		DWORD buf_len=0;
		BOOL ret= ReadDirectoryChangesW(	dir_handle,
											buf,
											buf_size,
											TRUE,							// Watch subtree
											FILE_NOTIFY_CHANGE_LAST_WRITE |
											FILE_NOTIFY_CHANGE_FILE_NAME,	// Filters
											&buf_len,						// Data size filled to buffer
											NULL,							// Asynchronous stuff
											NULL
											);
		if (ret){
			SizeType offset= 0;
			while (offset < buf_len){
				// Check every notify

				FILE_NOTIFY_INFORMATION& cur_info= *reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buf + offset);

				util::Str8 filename_relative_to_dir;


				// Add only even indices to convert utf16 to utf8 :-----D (WideCharToMultiByte?)
				for (uint32 c=0; c<cur_info.FileNameLength; c+=2)
					filename_relative_to_dir += ((char*)cur_info.FileName)[c];

				// Didn't work
				//filename_relative_to_dir= util::Str8("%.*ls", cur_info.FileNameLength/sizeof(WCHAR), cur_info.FileName);

				offset += cur_info.NextEntryOffset;

				// "Normalize"
				boost::filesystem::path boost_path;
				try {
					boost_path= boost::filesystem::canonical((dir_path + filename_relative_to_dir).cStr());
				}
				catch(...){
					print(debug::Ch::General, debug::Vb::Trivial, "Invalid path: %s", (dir_path + filename_relative_to_dir).cStr());
					continue; // Invalid path
				}
				
				util::Str8 complete_file_path= boost_path.string();
				print(debug::Ch::General, debug::Vb::Trivial, "File modified: %s", complete_file_path.cStr());

				{
					util::LockGuard<util::Mutex> lock(implMapMutex);

					auto handles= implMap[complete_file_path];

					//for (auto& m : implMap){
						//print(debug::Ch::General, debug::Vb::Trivial, "Watch: %s", m.first.cStr());
					//}

					FileWatcher::FileEvent e;

					if (cur_info.Action & FILE_ACTION_ADDED || cur_info.Action & FILE_ACTION_RENAMED_NEW_NAME)
						e= FileWatcher::FileEvent::Create;
					else if (cur_info.Action & FILE_ACTION_MODIFIED)
						e= FileWatcher::FileEvent::Modify;
					else
						e= FileWatcher::FileEvent::Delete;

					for (auto& m : handles){
						m->addLaunchable(e);
					}
				}

				if (offset == 0 || cur_info.NextEntryOffset == 0) break;
			}
		}
		else {
			print(debug::Ch::General, debug::Vb::Trivial, "Quitting FileWatcher thread");
			break;
		}
	}

	CloseHandle(dir_handle);
}

} // util
} // clover

#endif // OS_WINDOWS
