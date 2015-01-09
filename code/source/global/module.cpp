#include "module.hpp"

namespace clover {
namespace global {

Module::Module()
	: INIT_RESOURCE_ATTRIBUTE(name, "name", "")
	, INIT_RESOURCE_ATTRIBUTE(file, "file", "")
	, dll(nullptr)
{
	auto refresh= [&] ()
	{
		if (getResourceState() != State::Uninit)
			resourceUpdate(false);
	};

	file.setOnChangeCallback(refresh);
	file.get().setOnFileChangeCallback(
			[=] (resources::PathAttributeValue::FileEvent f){ refresh(); });
}

Module::~Module()
{ clear(); }

void* Module::getSym(const char* name) const
{ return hardware::queryDllSym(dll, name); }

void Module::resourceUpdate(bool load, bool force)
{
	if (getResourceState() == State::Uninit || load) {
		clear();
		file.get().setExt(hardware::dllExt());
		auto path= file.get().whole();
		print(	debug::Ch::Resources,
				debug::Vb::Trivial,
				"Loading DLL: %s",
				path.cStr());
		dll= hardware::loadDll(path.cStr());
		if (dll) {
			print(	debug::Ch::Resources,
					debug::Vb::Trivial,
					"DLL loaded at addr: %p",
					dll);
			setResourceState(State::Loaded);	
		} else {
			print(	debug::Ch::Resources,
					debug::Vb::Critical,
					"Couldn't load DLL: %s, %s", 
					path.cStr(), hardware::dllError());
			createErrorResource();
		}
	} else {
		auto path= file.get().whole();
		print(	debug::Ch::Resources,
				debug::Vb::Trivial,
				"Unloading DLL: %s",
				path.cStr());
		clear();
		setResourceState(State::Unloaded);
	}
}

void Module::createErrorResource()
{
	clear();
	setResourceState(State::Error);
}

void Module::clear()
{
	if (dll) {
		hardware::unloadDll(dll);
		dll= nullptr;
	}
}

} // global
} // clover
