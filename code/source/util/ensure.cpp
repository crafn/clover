#include "crashhandler.hpp"
#include "ensure.hpp"
#include "exception.hpp"
#include "misc.hpp"
#include "string.hpp"

namespace clover {
namespace util {

void ensureImpl(
		bool cond, const char* file, const char* func, int line,
		const char* cond_str)
{
	if (cond)
		return;

	/// @todo fail
	throw util::FatalException(
			"	in file %s\n	in method %s\n	  "
			"at line %i\n	  %s failed\nBacktrace:\n%s",
			util::getFilenameFromPath(file).c_str(),
			util::getReducedFunctionName(func).c_str(),
			line, cond_str, util::CrashHandler::getBacktrace(1).c_str());
}

void ensureMsgImpl(bool cond, const char* file, const char* func, int line,
		const char* format, ...)
{
	if (cond)
		return;

	va_list args;
    va_start(args, format);
	util::Str8 msg;
	msg.setFormattedArgList(format, args);
    va_end(args);

	/// @todo fail
	throw util::FatalException(
			"	in file %s\n	in method %s:\n	   at line %i\n	   %s\nBacktrace:\n%s",
			util::getFilenameFromPath(__FILE__).c_str(),
			util::getReducedFunctionName(__PRETTY_FUNCTION__).c_str(),
			line, msg.cStr(), util::CrashHandler::getBacktrace(1).c_str());
}

} // util

void fail(const char* format, ...)
{
	va_list args;
    va_start(args, format);
	util::Str8 msg;
	msg.setFormattedArgList(format, args);
    va_end(args);
	print(debug::Ch::General, debug::Vb::Critical, "fail: %s", msg.cStr());
	std::abort();
}

} // clover
