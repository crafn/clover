#include "ec_console.hpp"

namespace clover {
namespace game { namespace editor {

ConsoleEc::~ConsoleEc(){
}

void ConsoleEc::setVerbosity(debug::Vb v){
	debug::gDebugPrint.setVerbosity(v);
}

debug::Vb ConsoleEc::getVerbosity() const {
	return debug::gDebugPrint.getVerbosity();
}

void ConsoleEc::setChannelActive(debug::Ch id, bool b){
	debug::gDebugPrint.setChannelActive(id, b);
}

bool ConsoleEc::isChannelActive(debug::Ch id) const {
	return debug::gDebugPrint.isChannelActive(id);
}

void ConsoleEc::tryExecute(const util::Str8& string){
	try {
		scriptContext.execute(string);
	}
	catch (const script::ScriptException& e){
		print(debug::Ch::Dev, debug::Vb::Critical, "Console command error: %s", e.what());
	}
}

}} // game::editor
} // clover