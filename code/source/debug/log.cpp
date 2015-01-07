#include "log.hpp"

namespace clover {
namespace debug {

Log::Log(){
	file.open("debuglog.txt");
}

Log::~Log(){
	file.close();
}

void Log::operator()(const char8 * msg){
	if (file.good()){
		file << msg << "\n";
		file.flush();
	}
}

} // debug
} // clover
