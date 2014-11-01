#include "exception.hpp"

namespace clover {
namespace script {

ScriptException::ScriptException(const char* str, ...){
	va_list args;
	va_start(args, str);
	ResourceException::constructor(str, args);
	va_end(args);
}

ScriptException::~ScriptException() throw(){
}
	
} // script
} // clover