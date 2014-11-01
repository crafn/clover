#include "module.hpp"
#include "exception.hpp"
#include "objecttype.hpp"
#include "script_mgr.hpp"

#include <angelscript.h>
#include <scriptbuilder/scriptbuilder.h>

namespace clover {
namespace script {

Module::Module()
		: module(nullptr)
		, INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(fileAttribute, "file", ""){
		
	auto try_recreate= [&] () {
		if (getResourceState() != State::Uninit){
			create();
		}
	};
	
	fileAttribute.setOnChangeCallback([=] (){
		try_recreate();
	});
	
	fileAttribute.get().setOnFileChangeCallback([=] (resources::PathAttributeValue::FileEvent event){
		try_recreate();
	});
	
}

Module::~Module(){
	destroy();
}

void Module::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		create();
	}
	else {
		destroy();
	}
}

void Module::createErrorResource(){
	destroy();
	
	setResourceState(State::Error);
	module= gScriptMgr->getAsEngine().GetModule("Error");
	if (module) return;
	
	// Build error-module
	CScriptBuilder builder;
	int32 ret= builder.StartNewModule(&gScriptMgr->getAsEngine(), "Error"); ensure(ret >= 0);
	ret= builder.AddSectionFromMemory("error_section", ""); ensure(ret >= 0);
	ret= builder.BuildModule(); ensure(ret >= 0);
	
	module= gScriptMgr->getAsEngine().GetModule("Error");
	ensure(module);
}

ObjectType Module::getObjectType(const util::Str8& name) const {
	debug_ensure(module);
	asIObjectType* type= module->GetObjectTypeByName(Utils::leadingGlobalNamespaceRemoved(name).cStr());
	if (!type) throw ScriptException(
		"Script object type %s not found", name.cStr());
	return (ObjectType(*type));
}

asIScriptModule& Module::getAsModule() const {
	debug_ensure_msg(getResourceState() == State::Loaded || getResourceState() == State::Error, "state: %i", getResourceState());
	debug_ensure(module);
	return *module;
}

void Module::create(){
	destroy();
	
	try {
		build();
		setResourceState(State::Loaded);
	}
	catch (const resources::ResourceException&){
		createErrorResource();
	}
}

void Module::build(){
	
	CScriptBuilder builder;
	
	print(debug::Ch::Script, debug::Vb::Trivial, "Building module: %s", nameAttribute.get().cStr());
	
	int32 ret= builder.StartNewModule(&gScriptMgr->getAsEngine(), nameAttribute.get().cStr());
	Utils::errorCheck("Couldn't start script module", ret);
	
	ret= builder.AddSectionFromFile(fileAttribute.get().whole().cStr());
	Utils::errorCheck("Couldn't load script file: " + fileAttribute.get().whole(), ret);
	
	ret= builder.BuildModule();
	Utils::errorCheck("Couldn't build script module", ret);
	
	module= gScriptMgr->getAsEngine().GetModule(nameAttribute.get().cStr());
	Utils::errorCheck("Module " + nameAttribute.get() + " build failed", ret);
}

void Module::destroy(){
	if (module && getResourceState() != State::Error){ // Error-module is freed when ScriptMgr is destroyed
		print(debug::Ch::Script, debug::Vb::Trivial, "Unloading module: %s", nameAttribute.get().cStr());
		gScriptMgr->getAsEngine().DiscardModule(nameAttribute.get().cStr());
	}
	module= nullptr;
	
	setResourceState(State::Unloaded);
}

} // script
} // clover