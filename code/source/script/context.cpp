#include "context.hpp"
#include "exception.hpp"
#include "module.hpp"
#include "script_mgr.hpp"

#include <angelscript.h>

namespace clover {
namespace script {

Context::Context()
		: context(nullptr){
	context= gScriptMgr->getAsEngine().CreateContext();
}

Context::Context(Context&& other):
	context(other.context){
	other.context= nullptr;	
}

Context::Context(const Context& other):
	context(other.context){
	context->AddRef();
}

Context::~Context(){
	if (context)
		context->Release();
}

Object Context::instantiateObject(const ObjectType& type){
	ensure(context);
	
	if (!type.isGood())
		throw ScriptException("Trying to instantiate invalid script object type");
	
	asIObjectType& obj_type= type.getAsType();
	ensure_msg(type.getAsType().GetFactoryCount() == 1, "Factory count not 1 but %u for object %s (probably invalid object type)",
		obj_type.GetFactoryCount(), obj_type.GetName());
	
	asIScriptFunction* factory= obj_type.GetFactoryByIndex(0);
	ensure(factory);
	
	context->Prepare(factory);
	
	executeImpl();
	
	asIScriptObject* as_obj= *(asIScriptObject**)context->GetAddressOfReturnValue();
	ensure(as_obj);
	
	return Object(obj_type, *as_obj);
	
}

void Context::execute(const util::Str8& string){
	// Borrowed and modified from scripthelper.cpp from AngelScript addons
	
	// Wrap the code in a function so that it can be compiled and executed
	util::Str8 funcCode = "void executeString(){\n";
	funcCode += string;
	funcCode += "\n;}";
	
	// Dummy module
	asIScriptModule *execMod= script::gScriptMgr->getAsEngine().GetModule("__executeString", asGM_ALWAYS_CREATE);
	
	// Compile the function that can be executed
	asIScriptFunction *func= nullptr;
	int r= execMod->CompileFunction("executeString", funcCode.cStr(), -1, 0, &func);
	if(r < 0)
		throw ScriptException("Context::execute(..): Compile error: %s", script::Utils::retCodeToString(r).cStr());

	r= context->Prepare(func);
	if(r < 0){
		func->Release();
		throw ScriptException("Context::execute(..): Error in context->Prepare(func): %s", script::Utils::retCodeToString(r).cStr());
	}
	
	executeImpl();
	
	// Clean up
	func->Release();
}

bool Context::isFree() const {
	ensure(context);
	return context->GetState() == asEXECUTION_FINISHED || context->GetState() == asEXECUTION_UNINITIALIZED;
}

void Context::executeImpl(){
	gScriptMgr->pushExecutingContext(*this);
	int32 ret= context->Execute();
	gScriptMgr->popExecutingContext();
	executionErrorCheck(ret);
}

void Context::executionErrorCheck(int32 ret){
	if (ret != asEXECUTION_FINISHED){
		util::Str8 e_string= context->GetExceptionString();
		int32 e_line= context->GetExceptionLineNumber();
		
		// Create new context in the case of an error
		context->Release();
		context= gScriptMgr->getAsEngine().CreateContext();
		
		if (ret == asEXECUTION_EXCEPTION){
			throw ScriptException(	"Script runtime exception: %s\nline: %i",
									e_string.cStr(),
									e_line);
		}
		else {
			throw ScriptException("Error while executing script: %s", Utils::retCodeToString(ret).cStr());
		}
		
	}
}

} // script
} // clover