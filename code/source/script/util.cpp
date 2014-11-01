#include "util.hpp"
#include "exception.hpp"
#include "util/ensure.hpp"

#include <angelscript.h>

namespace clover {
namespace script {

util::Str8 Utils::retCodeToString(int32 code){
	switch (code){
	case asSUCCESS: return "asSUCCESS";
	case asERROR: return "asERROR";
	case asCONTEXT_ACTIVE: return "asCONTEXT_ACTIVE";
	case asCONTEXT_NOT_FINISHED: return "asCONTEXT_NOT_FINISHED";
	case asCONTEXT_NOT_PREPARED: return "asCONTEXT_NOT_PREPARED";
	case asINVALID_ARG: return "asINVALID_ARG";
	case asNO_FUNCTION: return "asNO_FUNCTION";
	case asNOT_SUPPORTED: return "asNOT_SUPPORTED";
	case asINVALID_NAME: return "asINVALID_NAME";
	case asNAME_TAKEN: return "asNAME_TAKEN";
	case asINVALID_DECLARATION: return "asINVALID_DECLARATION";
	case asINVALID_OBJECT: return "asINVALID_OBJECT";
	case asINVALID_TYPE: return "asINVALID_TYPE";
	case asALREADY_REGISTERED: return "asALREADY_REGISTERED";
	case asMULTIPLE_FUNCTIONS: return "asMULTIPLE_FUNCTIONS";
	case asNO_MODULE: return "asNO_MODULE";
	case asNO_GLOBAL_VAR: return "asNO_GLOBAL_VAR";
	case asINVALID_CONFIGURATION: return "asINVALID_CONFIGURATION";
	case asINVALID_INTERFACE: return "asINVALID_INTERFACE";
	case asCANT_BIND_ALL_FUNCTIONS: return "asCANT_BIND_ALL_FUNCTIONS";
	case asLOWER_ARRAY_DIMENSION_NOT_REGISTERED: return "asLOWER_ARRAY_DIMENSION_NOT_REGISTERED";
	case asWRONG_CONFIG_GROUP: return "asWRONG_CONFIG_GROUP";
	case asCONFIG_GROUP_IS_IN_USE: return "asCONFIG_GROUP_IS_IN_USE";
	case asILLEGAL_BEHAVIOUR_FOR_TYPE: return "asILLEGAL_BEHAVIOUR_FOR_TYPE";
	case asWRONG_CALLING_CONV: return "asWRONG_CALLING_CONV";
	case asBUILD_IN_PROGRESS: return "asBUILD_IN_PROGRESS";
	case asINIT_GLOBAL_VARS_FAILED: return "asINIT_GLOBAL_VARS_FAILED";
	case asOUT_OF_MEMORY: return "asOUT_OF_MEMORY";
	default: return util::Str8::format("Unknown return code: %i", code);
	}
}

void Utils::errorCheck(const util::Str8& msg, int32 code){
	if (code < 0) throw ScriptException("%s: %s", msg.cStr(), Utils::retCodeToString(code).cStr());
}	

void Utils::devErrorCheck(const util::Str8& msg, int32 code){
	#ifndef RELEASE
	if (code < 0) throw ScriptException("%s: %s", msg.cStr(), Utils::retCodeToString(code).cStr());
	#endif
}

util::Str8 Utils::leadingGlobalNamespaceRemoved(util::Str8 name){
	if (name.length() < 2)
		return name;
	
	if (name[0] == ':' && name[1] == ':'){
		util::Str8 ret= name;
		ret.erase(0, 2);
		return ret;
	}
	else {
		return name;
	}
}

util::Str8 Utils::leadingNamespaceRemoved(util::Str8 name){
	//print(debug::Ch::Script, debug::Vb::Trivial, "IN: %s", name.cStr());
	
	util::DynArray<util::Str8> parts= name.splitted(':', 2);
	ensure(!parts.empty());
	
	if (parts.size() == 1 || parts[0].count('<') > 0)
		return name;
	
	util::Str8 ret;
	for (SizeType i= 1; i < parts.size(); ++i){
		ret += parts[i];
		if (i + 1 < parts.size())
			ret += "::";
	}

	//print(debug::Ch::Script, debug::Vb::Trivial, "OUT: %s", ret.cStr());
	
	return ret;
}

} // script
} // clover