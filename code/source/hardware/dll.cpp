#include "dll.hpp"

#if OS == OS_LINUX
#	include <dlfcn.h>
#else
#	error @todo Implement
#endif

namespace clover {
namespace hardware {

#if OS == OS_LINUX

DllHandle loadDll(const char* path)
{ return dlopen(path, RTLD_LAZY); }

void unloadDll(DllHandle dll)
{ dlclose(dll); }

void* queryDllSym(DllHandle dll, const char* sym)
{ return dlsym(dll, sym); }

const char* dllError()
{ return dlerror(); }

#else
#	error @todo Implement
#endif

} // hardware
} // clover
