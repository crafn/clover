#ifndef CLOVER_DEBUGLOG_HPP
#define CLOVER_DEBUGLOG_HPP

#include "build.hpp"

#include <fstream>

namespace clover {
namespace debug {

/// Performs logging to file
class DebugLog {
	std::ofstream file;
public:
	DebugLog();
	virtual ~DebugLog();
	
	/// Appends row
	void operator()(const char8 * msg);
};

extern DebugLog gDebugLog;

} // debug
} // clover

#endif // CLOVER_DEBUGLOG_HPP
