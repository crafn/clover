#include "debugprint.hpp"
#include "debug/debuglog.hpp"
#include "global/event.hpp"
#include "util/ensure.hpp"
#include "util/misc.hpp"
#include "util/mutex.hpp"
#include "util/string.hpp"
#include <stdarg.h>
#include <iostream>

namespace clover {
namespace debug {

DebugPrint gDebugPrint;

util::Str8& DebugPrint::getChannelString(Ch id){
	static util::Str8 str[channelCount]= {
		"Anim",
		"WE",
		"OpenGL",
		"OpenCL",
		"Device",
		"Phys",
		"Audio",
		"Event",
		"Visual",
		"Resources",
		"Save",
		"Gui",
		"Ui",
		"WorldChunk",
		"Script",
		"Net",
		"Nodes",
		"Dev",
		"General"
	};
	ensure(static_cast<SizeType>(id) < channelCount);
	return str[static_cast<SizeType>(id)];
}

util::Str8& DebugPrint::getVerbosityString(Vb v){
	static util::Str8 str[verbosityCount]= {
		"Trivial",
		"Moderate",
		"Critical",
	};
	ensure(static_cast<SizeType>(v) < verbosityCount);
	return str[static_cast<SizeType>(v)];
}

DebugPrint::DebugPrint(){
	verbosity= Vb::Trivial;
	filter.reset();
	filter.flip();
}

void DebugPrint::setVerbosity(Vb v){
	util::LockGuard<util::Mutex> lock(mutex);
	verbosity= v;
}

void DebugPrint::setChannelActive(Ch c, bool b){
	ensure(static_cast<SizeType>(c) < channelCount);
	util::LockGuard<util::Mutex> lock(mutex);
	filter.set(static_cast<SizeType>(c), b);
}

bool DebugPrint::isChannelActive(Ch c) const {
	ensure(static_cast<SizeType>(c) < channelCount);
	util::LockGuard<util::Mutex> lock(mutex);
	return filter.test(static_cast<SizeType>(c));
}

void DebugPrint::operator()(Ch c, Vb v, const char8 * str, ...){
	util::LockGuard<util::Mutex> lock(mutex);

	va_list argList;
	va_start(argList, str);
	util::Str8 msg_str;
	msg_str.setFormattedArgList(str, argList);
	va_end(argList);
	
	if (v == Vb::Critical)
		msg_str= "*** " + msg_str;
	
	//Debuggifileeseen menee kaikki tulostus
	gDebugLog(msg_str.cStr());

	if (	static_cast<SizeType>(v) < static_cast<SizeType>(verbosity) ||
			!filter.test(static_cast<SizeType>(c)))
		return;

	std::cout << msg_str.cStr() << "\n";
	
	Message message;
	message.channel= c;
	message.verbosity= v;
	message.string= msg_str.cStr();
	
	buffer.push_back(std::move(message));
}

void DebugPrint::updateBuffer(){
	util::LockGuard<util::Mutex> lock(mutex);
	if (!buffer.empty()){
		global::Event e(global::Event::OnConsoleBufferUpdate);
		e(global::Event::Object)= &buffer;
		e.send();
		
		buffer.clear();
	}
}

} // debug
} // clover