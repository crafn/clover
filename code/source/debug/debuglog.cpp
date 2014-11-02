#include "debuglog.hpp"

namespace clover {
namespace debug {

DebugLog gDebugLog;

DebugLog::DebugLog(){
	file.open("debuglog.txt");
}

DebugLog::~DebugLog(){
	file.close();
}

void DebugLog::operator()(const char8 * msg){
	if (file.good()){
		file << msg << "\n";
		file.flush();
	}
}

} // debug
} // clover