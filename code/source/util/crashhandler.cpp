#include "crashhandler.hpp"
#include "string.hpp"

#include <cstdlib>
#include <exception>
#include <fstream>
#include <iostream>

#if OS == OS_WINDOWS
	#include <signal.h>
	#include <windows.h>
#elif OS == OS_LINUX
	#include <execinfo.h>
	#include <signal.h>
	#include <stdio.h>
	#include <unistd.h>
#endif

namespace clover {
namespace util {

CrashHandler::CrashHandler(){
	registerSignalHandler();
}

CrashHandler::~CrashHandler(){
}

void CrashHandler::onUnhandledException(const std::exception& e){
	std::ofstream file(getOutputFilePath().c_str(), std::ios::binary);
	if (!file) std::cerr << "Couldn't open crash report file\n";
	file << getBuildStr() << std::endl;
	file << "Unhandled exception:\n";
	file << e.what() << std::endl;
}

std::string CrashHandler::getOutputFilePath(){
	util::Str8 basepath= "crashreport";
	
	int32 num= 0;
	// Find unique filename
	while (std::ifstream(util::Str8::format("%s%i.txt", basepath.cStr(), num).cStr())){
		++num;
	}
	
	return std::string(util::Str8::format("%s%i.txt", basepath.cStr(), num).cStr());
}

// Platform specifics

#if OS == OS_WINDOWS

	std::string CrashHandler::getBacktrace(uint32 frames_to_skip)
	{
		util::Str8 ret;



		// MSDN: Windows Server 2003 and Windows XP:  The sum of the FramesToSkip and FramesToCapture parameters must be less than 63.
		const ULONG max_count= 62;
		LPVOID trace[max_count];
		DWORD hash_code;

		typedef USHORT (WINAPI *CaptureStackBackTraceType)(ULONG, ULONG, PVOID*, PULONG);
		CaptureStackBackTraceType backtrace= (CaptureStackBackTraceType)(GetProcAddress(LoadLibrary("kernel32.dll"), "RtlCaptureStackBackTrace"));

		if (backtrace){
			// Use -fno-omit-frame-pointer flag to get more than one trace
			int trace_size= backtrace(frames_to_skip, max_count, trace, &hash_code);
			for (int i=0; i<trace_size; ++i){
				ret += util::Str8::format("%i: %p\n", i, trace[i]);
			}
		}
		else {
			ret += "Couldn't find CaptureStackBackTrace\n";
		}
		return std::string(ret.cStr());
	}

	void CrashHandler::registerSignalHandler(){
		signal(SIGABRT, &onSignal);
		signal(SIGSEGV, &onSignal);
	}

	void CrashHandler::onSignal(int sig){
		{
			std::ofstream file(getOutputFilePath().c_str(), std::ios::binary);
			if (!file) std::cerr << "Couldn't open crash report file\n";
			file << getBuildStr() << "\n";

			if (sig == SIGABRT)
				file << "SIGABRT received\n";
			else if (sig == SIGSEGV)
				file << "SIGSEGV received\n";
			else
				file << "Signal received: " << sig;

			file << "Backtrace:\n";
			file << getBacktrace();
		}
		exit(0);
	}

#elif OS == OS_LINUX

	std::string CrashHandler::getBacktrace(uint32 frames_to_skip)
	{
		const int max_count= 128;
		void* trace[max_count];
		int trace_size= backtrace(trace, max_count);

		char** messages= backtrace_symbols(trace, trace_size);

		util::Str8 ret;
		for (int i= frames_to_skip; i < trace_size; ++i){
			ret += util::Str8::format("#%i %s\n", i, messages[i]);
		}

		return (std::string(ret.cStr()));
	}

	void CrashHandler::registerSignalHandler(){
		struct sigaction s;
		s.sa_sigaction= &onSignal;
		sigemptyset(&s.sa_mask);
		s.sa_flags= SA_RESTART | SA_SIGINFO;
		sigaction(SIGABRT, &s, nullptr);
		sigaction(SIGSEGV, &s, nullptr);
	}

	void CrashHandler::onSignal(int sig, siginfo_t* info, void* secret){
		{
			std::ofstream file(getOutputFilePath().c_str(), std::ios::binary);
			if (!file) std::cerr << "Couldn't open crash report file\n";

			file << getBuildStr() << "\n";

			if (sig == SIGSEGV)
				file << util::Str8::format("SIGSEGV received, si_addr: %p", info->si_addr).cStr() << "\n";
			else
				file << util::Str8::format("Signal received: %i", sig).cStr() << "\n";

			file << "Backtrace\n";
			file << getBacktrace();

		}
		exit(0);
	}

#endif

} // util
} // clover
