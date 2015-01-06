#include "dll.hpp"
#include "util/string.hpp"

#if PLATFORM == PLATFORM_UNIX
#	include <dlfcn.h>
#elif PLATFORM == PLATFORM_WINDOWS
#	include <windows.h>
#endif

namespace clover {
namespace hardware {

#if PLATFORM == PLATFORM_UNIX

DllHandle loadDll(const char* path)
{ return dlopen(path, RTLD_LAZY | RTLD_GLOBAL); }

void unloadDll(DllHandle dll)
{ dlclose(dll); }

void* queryDllSym(DllHandle dll, const char* sym)
{ return dlsym(dll, sym); }

const char* dllError()
{ return dlerror(); }

const char* dllExt()
{ return "so"; }

#elif PLATFORM == PLATFORM_WINDOWS

DllHandle loadDll(const char* path)
{ return static_cast<DllHandle>(LoadLibrary(path)); }

void unloadDll(DllHandle dll)
{ FreeLibrary(static_cast<HMODULE>(dll)); }

void* queryDllSym(DllHandle dll, const char* sym)
{ return (void*)GetProcAddress(static_cast<HMODULE>(dll), sym); }

const char* dllError()
{ return "@todo dllError on windows"; }

const char* dllExt()
{ return "dll"; }
#endif

} // hardware
} // clover
