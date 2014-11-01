#ifndef CLOVER_UTIL_CRASHHANDLER_HPP
#define CLOVER_UTIL_CRASHHANDLER_HPP

#include "build.hpp"

#if OS == OS_LINUX
	#include <ucontext.h>
#endif

#include <stdexcept>
#include <string>

namespace clover {
namespace util {

class CrashHandler {
public:
	CrashHandler();
	~CrashHandler();
	
	void onUnhandledException(const std::exception& e);
	
	static std::string getBacktrace();
	
private:
	static std::string getOutputFilePath();

	void registerSignalHandler();

	// Platform specific stuff
	#if OS == OS_WINDOWS
		static void onSignal(int sig);
	#elif OS == OS_LINUX
		static void onSignal(int sig, siginfo_t* info, void* secret);
	#endif
};

} // util
} // clover

#endif // CLOVER_UTIL_CRASHHANDLER_HPP