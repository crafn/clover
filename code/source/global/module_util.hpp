#ifndef CLOVER_GLOBAL_MODULE_UTIL_HPP
#define CLOVER_GLOBAL_MODULE_UTIL_HPP

#include "build.hpp"

namespace clover {
namespace hardware { using DllHandle= void*; }
namespace global {

/** Example usage

// my_own_module.cpp
DECLARE_MOD(my_module) // Present in every file using MAKE_MOD_FPTR
DEFINE_MOD(my_module) // Present only in one .cpp

MOD_API void callback(int i)
{ debug::print("hello from mod: %i", i); }

MOD_API void setThings(EngineStuff* e)
{ e->callback= MAKE_MOD_FPTR(callback); }


// somewhere_in_engine.hpp
struct ENGINE_API EngineStuff {
	MOD_FPTR_TYPE(void (*)(int)) callback; // Module-reload -friendly fptr
}; 

void doEngineStuff(EngineStuff* e)
{
	setThings(e);
	(*e->callback)(1337); // "hello from mod: 1337"
}

*/

/// This should be used at least in once in a module
#define DECLARE_MOD(name) \
	MOD_API clover::global::PersistentModInfo* mod_info

/// This should be used exactly once in a module (in a single translation unit)
#define DEFINE_MOD(name) \
	clover::global::PersistentModInfo* mod_info= nullptr // Main program sets


/// @todo Build option to substitute safe ModuleFPtrs with plain pointers
///       as it's the desired feature for release builds

/// Returns a "function pointer" which supports relocation on module reload
/// Use this when module supplies a persistent function pointer to engine
/// @warning Not thread safe!
#define MAKE_MOD_FPTR(ptr) \
	clover::global::makeModuleFPtr(ptr, #ptr, mod_info)

/// Makes a "function pointer" type which does automatic relocation on module
/// reload
/// @warning Not thread safe!
#define MOD_FPTR_TYPE(ptr_type) \
	clover::global::ModuleFPtr<ptr_type>



// Implementation

/// Instances remain in memory even though modules are reloaded
struct PersistentModInfo {
	const char* path;
	hardware::DllHandle handle;
	bool valid;
};

/// Makes relocation of a function pointer possible even if module is reloaded
/// at runtime to different memory location
template <typename Ptr>
struct ModuleFPtr {
	const char* sym; /// Pointer to persistent storage
	mutable PersistentModInfo* modInfo; /// Pointer to persistent storage
	mutable Ptr ptr; /// Function pointer to module -- can be invalid

	ModuleFPtr()= default;
	ModuleFPtr(const ModuleFPtr&)= default;
	ModuleFPtr(Ptr ptr);
	ModuleFPtr& operator=(const ModuleFPtr&)= default;
	ModuleFPtr& operator=(Ptr ptr);

	explicit operator bool() const;
	Ptr operator*() const;

	template <typename Ptr2>
	explicit operator ModuleFPtr<Ptr2>() const;

	/// @todo operator() -- now dereferencing must be used to call
};
using VoidFPtr= void (*)();
using VoidModuleFPtr= ModuleFPtr<VoidFPtr>;

void onModuleLoad(const char* path, hardware::DllHandle handle);
void onModuleUnload(hardware::DllHandle handle);

/// @todo Replace these when using my own template generation

ENGINE_API void makeModuleFPtr(VoidModuleFPtr* call, VoidFPtr ptr, const char* sym, PersistentModInfo* info);
template <typename Ptr>
ModuleFPtr<Ptr> makeModuleFPtr(Ptr ptr, const char* sym, PersistentModInfo* info)
{ ModuleFPtr<Ptr> call; makeModuleFPtr((VoidModuleFPtr*)&call, (VoidFPtr)ptr, sym, info); return call; }

ENGINE_API VoidFPtr relocatedModuleFPtr(const VoidModuleFPtr* call);
template <typename Ptr>
Ptr relocatedModuleFPtr(const ModuleFPtr<Ptr>* call)
{ return (Ptr)relocatedModuleFPtr((const VoidModuleFPtr*)call); }

ENGINE_API void copyModuleFPtr(char* dst, char* src);

template <typename Ptr>
ModuleFPtr<Ptr>::ModuleFPtr(Ptr ptr)
{ operator=(ptr); }

template <typename Ptr>
ModuleFPtr<Ptr>& ModuleFPtr<Ptr>::operator=(Ptr p)
{
	sym= nullptr;
	modInfo= nullptr;
	ptr= p;
	return *this;
}

template <typename Ptr>
ModuleFPtr<Ptr>::operator bool() const
{ return relocatedModuleFPtr(this) != nullptr; }

template <typename Ptr>
Ptr ModuleFPtr<Ptr>::operator*() const
{ return relocatedModuleFPtr(this); }

template <typename Ptr>
template <typename Ptr2>
ModuleFPtr<Ptr>::operator ModuleFPtr<Ptr2>() const
{
	ModuleFPtr<Ptr2> other;
	// I think they should always be layout-compatible
	copyModuleFPtr((char*)&other, (char*)this);
	return other;
}

} // global
} // clover

#endif // CLOVER_GLOBAL_MODULE_UTIL_HPP
