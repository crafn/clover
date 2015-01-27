#include "debug/print.hpp"
#include "global/env.hpp"
#include "hardware/device.hpp"
#include "module.hpp"
#include "module_util.hpp"

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
			resourceUpdate(true);
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

		// Can't continue without dll being (re)loaded as we don't want
		// function pointers to dll become invalid
		while (!dll) {
			print(	debug::Ch::Resources,
					debug::Vb::Trivial,
					"Loading DLL: %s",
					path.cStr());

			dll= hardware::loadDll(path.cStr());
			if (!dll) {
				print(	debug::Ch::Resources,
						debug::Vb::Critical,
						"Couldn't load a DLL: %s, %s\nTrying again...", 
						path.cStr(), hardware::dllError());
				global::g_env.device->sleep(0.1);
			}
		}

		print(debug::Ch::Resources, debug::Vb::Trivial, "DLL loaded");
		onModuleLoad(path.cStr(), dll);

		// Notify change-listeners
		// This behaviour is quite tightly coupled with the listeners;
		// They expect module to always resolve its status to `Loaded`
		// before letting main thread to continue elsewhere
		setResourceState(State::Unloaded);
		setResourceState(State::Loaded);
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
		onModuleUnload(dll);
		hardware::unloadDll(dll);
		dll= nullptr;
	}
}

} // global
} // clover
