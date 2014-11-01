#include "exception.hpp"

namespace clover {
namespace net {

NetException::NetException(const char* str, ...){
	va_list args;
	va_start(args, str);
	global::Exception::constructor(str, args);
	va_end(args);
}

} // net
} // clover