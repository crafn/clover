#include "exception.hpp"
#include "debug/print.hpp"
#include "util/string.hpp"

namespace clover {
namespace util {

Exception::Exception(const char* s, ...){
	va_list argList;
	va_start(argList, s);

	constructor(s, argList);

	va_end(argList);
}

Exception::Exception(const char8* s, va_list arglist){
	constructor(s, arglist);
}

Exception::Exception(){
}

void Exception::constructor(const char8* s, va_list arglist){
	/// @todo This is bad
	static util::Str8 er;
	er.setFormattedArgList(s, arglist);
	error= er.cStr();

	print(debug::Ch::General, debug::Vb::Critical, "Exception thrown: %s", error);
}

Exception::~Exception() throw() {
}

const char* Exception::what() const throw() {
	return error;
}

FatalException::FatalException(const char* str, ...){
	va_list args;
	va_start(args, str);
	Exception::constructor(str, args);
	va_end(args);
}

} // util
} // clover
