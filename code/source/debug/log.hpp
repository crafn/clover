#ifndef CLOVER_DEBUGLOG_HPP
#define CLOVER_DEBUGLOG_HPP

#include "build.hpp"

#include <fstream>

namespace clover {
namespace debug {

/// Performs logging to file
class Log {
public:
	Log();
	virtual ~Log();

	/// Appends row
	void operator()(const char8 * msg);

private:
	std::ofstream file;
};

} // debug
} // clover

#endif // CLOVER_DEBUGLOG_HPP
