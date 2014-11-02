#ifndef CLOVER_UTIL_ENSURE_HPP
#define CLOVER_UTIL_ENSURE_HPP

#include "crashhandler.hpp"
#include "exception.hpp"
#include "misc.hpp"

#define ensure_impl(x) if (! (x)) { \
				throw util::FatalException("	in file %s\n	in method %s\n	  at line %i\n	  %s failed\nBacktrace:\n%s",\
								util::getFilenameFromPath(__FILE__).c_str(),\
								util::getReducedFunctionName(__PRETTY_FUNCTION__).c_str(),\
								__LINE__, #x, util::CrashHandler::getBacktrace().c_str()); }
								
#define ensure_msg_impl(x, a, ...) if (!(x)){\
				throw util::FatalException("	in file %s\n	in method %s:\n	   at line %i\n	   " a "\nBacktrace:\n%s",\
								util::getFilenameFromPath(__FILE__).c_str(),\
								util::getReducedFunctionName(__PRETTY_FUNCTION__).c_str(),\
								__LINE__, ##__VA_ARGS__, util::CrashHandler::getBacktrace().c_str()); }

#define		release_ensure(x) ensure_impl(x)
#define		release_ensure_msg(x, a, ...) ensure_msg_impl(x, a, ##__VA_ARGS__)


#ifndef RELEASE
	// Development assertti
	#define		ensure(x) ensure_impl(x)
	#define		ensure_msg(x, a, ...) ensure_msg_impl(x, a, ##__VA_ARGS__)

#else

	#define		ensure(x)
	#define		ensure_msg(x, a, ...)

#endif

#ifdef DEBUG
	#define		debug_ensure(x) ensure_impl(x)
	#define		debug_ensure_msg(x, a, ...) ensure_msg_impl(x, a, ##__VA_ARGS__)
#else
	#define		debug_ensure(x)
	#define		debug_ensure_msg(x, a, ...)
#endif

namespace clover {
namespace util {

template <typename T>
T* noNullFlowsThrough(T* ptr){
	release_ensure_msg(ptr != nullptr, "nullptr");
	return ptr;
}

} // util
} // clover

#ifndef RELEASE
#define NONULL(ptr) util::noNullFlowsThrough(ptr)
#else
#define NONULL(ptr) ptr
#endif

#endif // CLOVER_UTIL_ENSURE_HPP