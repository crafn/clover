#include "global/module_util.hpp"
#include "hardware/dll.hpp"
#include "util/ensure.hpp"
#include "util/profiling.hpp"

#include <cstring>

namespace clover {
namespace util {

// Move to own header if needed elsewhere
char* strdup(const char* src)
{
	SizeType byte_count= std::strlen(src) + 1;
	char* dst= new char[byte_count];
	std::memcpy(dst, src, byte_count);
	return dst;
}

} // util
namespace global {

const SizeType maxModInfos= 2048;
const SizeType maxModSyms= maxModInfos*4;

/// @todo Don't leak const char*s stored in these
static PersistentModInfo g_modInfos[maxModInfos]= {};
static SizeType g_nextModInfo= 0;
static const char* g_modSymStorage[maxModSyms]= {};

void onModuleLoad(const char* path, hardware::DllHandle handle)
{
	if (g_nextModInfo >= maxModInfos)
		fail("Maximum module count/reloads exceeded");

	PersistentModInfo info;
	info.path= util::strdup(path);
	info.handle= handle;
	info.valid= true;
	g_modInfos[g_nextModInfo]= info;

	auto mod_info_ptr=
		(PersistentModInfo**)hardware::queryDllSym(handle, "mod_info"); 
	if (!mod_info_ptr) {
		fail(	"PersistentModInfo missing for module: %s\n"
				"Module should contain DECLARE_MOD(name) and DEFINE_MOD(name)",
				path);
	}

	*mod_info_ptr= &g_modInfos[g_nextModInfo];
	++g_nextModInfo;
}

void onModuleUnload(hardware::DllHandle handle)
{
	for (SizeType i= 0; i < g_nextModInfo; ++i) {
		if (g_modInfos[i].handle == handle) {
			g_modInfos[i].valid= false;
		}
	}
}

void makeModuleFPtr(
		VoidModuleFPtr* call,
		VoidFPtr ptr,
		const char* sym,
		PersistentModInfo* info)
{
	ensure(info);

	call->sym= nullptr;
	for (SizeType i= 0; i < maxModSyms; ++i) {
		if (g_modSymStorage[i] == nullptr) {
			// No symbol found, allocate new
			call->sym= g_modSymStorage[i]= util::strdup(sym);
			break;
		} else if (!std::strcmp(g_modSymStorage[i], sym)) {
			// Symbol name already allocated
			call->sym= g_modSymStorage[i];
			break;
		}
	}
	if (!call->sym)
		fail("Too many module symbols present");

	call->modInfo= info;
	call->ptr= ptr;
}

VoidFPtr relocatedModuleFPtr(const VoidModuleFPtr* call)
{
	if (!call->modInfo || !call->sym)
		return call->ptr; // This is not pointing to a module func

	if (call->modInfo->valid) {
		// Module hasn't been relocated
		return call->ptr;
	} else {
		// Relocate
		const char* path= call->modInfo->path;
		PersistentModInfo* info= nullptr;
		for (auto& mod : g_modInfos) {
			if (mod.valid && !std::strcmp(path, mod.path)) {
				info= &mod;
			}
		}
		if (!info)
			fail("Calling function of a missing module: %s", call->modInfo->path);

		call->modInfo= info;
		call->ptr= (VoidFPtr)hardware::queryDllSym(info->handle, call->sym);
		ensure(call->modInfo->valid);

		return relocatedModuleFPtr(call);
	}
}

void copyModuleFPtr(char* dst, char* src)
{
	std::memcpy(dst, src, sizeof(VoidModuleFPtr));
}

} // global
} // clover
