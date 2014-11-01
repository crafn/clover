#include "app.hpp"
#include "build.hpp"
#include "util/crashhandler.hpp"
#include "util/profiler.hpp"

#include <exception>

int main(int, char *argv[]){
	clover::util::CrashHandler crashHandler;
	
	try {
		clover::App app(argv[0]);
		app.run();	
	}
	catch (const std::exception& e){
		crashHandler.onUnhandledException(e);
	}
	
	return 0;
}
