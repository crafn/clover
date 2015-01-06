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

DllHandle loadDll(const char* path_without_ext)
{
	auto path= util::Str8::format("%s.so", path_without_ext);
	return dlopen(path.cStr(), RTLD_LAZY | RTLD_GLOBAL);
}

void unloadDll(DllHandle dll)
{ dlclose(dll); }

void* queryDllSym(DllHandle dll, const char* sym)
{ return dlsym(dll, sym); }

const char* dllError()
{ return dlerror(); }

#elif PLATFORM == PLATFORM_WINDOWS

DllHandle loadDll(const char* path)
{ return static_cast<DllHandle>(LoadLibrary(path)); }

void unloadDll(DllHandle dll)
{ FreeLibrary(static_cast<HMODULE>(dll)); }

void* queryDllSym(DllHandle dll, const char* sym)
{ return (void*)GetProcAddress(static_cast<HMODULE>(dll), sym); }

const char* dllError()
{ return "@todo dllError on windows"; }

#endif

} // hardware
} // clover
