#ifndef CLOVER_SCRIPT_UTIL_HPP
#define CLOVER_SCRIPT_UTIL_HPP

#include "build.hpp"
#include "util/string.hpp"

namespace clover {
namespace script {

/// @todo Remove useless struct but preserve functions
struct Utils {
	
	static util::Str8 retCodeToString(int32 code);
	
	/// Throws ScriptException in case of error
	static void errorCheck(const util::Str8& msg, int32 code);
	static void devErrorCheck(const util::Str8& msg, int32 code);
	
	/// "::Type" -> "Type"
	/// "physics::Object" -> "physics::Object"
	static util::Str8 leadingGlobalNamespaceRemoved(util::Str8 name);
	
	/// "::Type" -> "Type"
	/// "physics::Object" -> "Object"
	/// "::Array<physics::Object>" -> "Array<physics::Object>"
	static util::Str8 leadingNamespaceRemoved(util::Str8 name);
};
	
} // script
} // clover

#endif // CLOVER_SCRIPT_UTIL_HPP