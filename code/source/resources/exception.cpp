#include "exception.hpp"

namespace clover {
namespace resources {

ResourceException::ResourceException(const char* str, ...){
	va_list args;
	va_start(args, str);
	global::Exception::constructor(str, args);
	va_end(args);
}

ResourceException::ResourceException(){}

ResourceException::~ResourceException() throw() {
	
}
	
} // resources
} // clover