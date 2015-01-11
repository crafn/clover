#ifndef CLOVER_UTIL_ENSURE_HPP
#define CLOVER_UTIL_ENSURE_HPP

#include "build.hpp"

#define ensure_impl(x) \
	util::ensureImpl(!!(x), __FILE__, __PRETTY_FUNCTION__, __LINE__, #x)

#define ensure_msg_impl(x, a, ...) \
	util::ensureMsgImpl(!!(x), __FILE__, __PRETTY_FUNCTION__, __LINE__, a, ##__VA_ARGS__)

#define		release_ensure(x) ensure_impl(x)
#define		release_ensure_msg(x, a, ...) ensure_msg_impl(x, a, ##__VA_ARGS__)


#ifndef RELEASE
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

ENGINE_API void
ensureImpl(bool cond, const char* file, const char* func, int line,
		const char* cond_str);
ENGINE_API void
ensureMsgImpl(bool cond, const char* file, const char* func, int line,
		const char* msg, ...);

template <typename T>
T* noNullFlowsThrough(T* ptr){
	release_ensure_msg(ptr != nullptr, "nullptr");
	return ptr;
}

} // util

void fail(const char* format, ...);

} // clover

#ifndef RELEASE
#define NONULL(ptr) util::noNullFlowsThrough(ptr)
#else
#define NONULL(ptr) ptr
#endif

#endif // CLOVER_UTIL_ENSURE_HPP
