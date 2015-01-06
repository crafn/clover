#ifndef CLOVER_HARDWARE_DLL_HPP
#define CLOVER_HARDWARE_DLL_HPP

#include "build.hpp"

namespace clover {
namespace hardware {

using DllHandle= void*;
const DllHandle mainProgramHandle= nullptr;

DllHandle loadDll(const char* path_without_ext);
void unloadDll(DllHandle dll);
void* queryDllSym(DllHandle dll, const char* sym);
const char* dllError();

} // hardware
} // clover

#endif // CLOVER_HARDWARE_DLL_HPP
