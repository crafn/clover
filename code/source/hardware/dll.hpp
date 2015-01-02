#ifndef CLOVER_HARDWARE_DLL_HPP
#define CLOVER_HARDWARE_DLL_HPP

#include "build.hpp"

namespace clover {
namespace hardware {

#if OS == OS_LINUX
#define DLL_EXPORT extern "C" __attribute__((visibility ("default")))
using DllHandle= void*;
#else
#error @todo Implement
#endif

const DllHandle mainProgramHandle= nullptr;

DllHandle loadDll(const char* path);
void unloadDll(DllHandle dll);
void* queryDllSym(DllHandle dll, const char* sym);
const char* dllError();

} // hardware
} // clover

#endif // CLOVER_HARDWARE_DLL_HPP
