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
		dll= hardware::loadDll(path.cStr());
		if (dll) {
			setResourceState(State::Loaded);	
		} else {
			print(	debug::Ch::Resources,
					debug::Vb::Critical,
					"Couldn't load DLL: %s, %s", 
					path.cStr(), hardware::dllError());
			createErrorResource();
		}
	} else {
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
