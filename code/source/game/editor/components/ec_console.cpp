#include "ec_console.hpp"
#include "debug/print.hpp"

namespace clover {
namespace game { namespace editor {

ConsoleEc::~ConsoleEc(){
}

void ConsoleEc::setVerbosity(debug::Vb v){
	global::g_env->debugPrint->setVerbosity(v);
}

debug::Vb ConsoleEc::getVerbosity() const {
	return global::g_env->debugPrint->getVerbosity();
}

void ConsoleEc::setChannelActive(debug::Ch id, bool b){
	global::g_env->debugPrint->setChannelActive(id, b);
}

bool ConsoleEc::isChannelActive(debug::Ch id) const {
	return global::g_env->debugPrint->isChannelActive(id);
}

void ConsoleEc::tryExecute(const util::Str8& string){
	print(debug::Ch::Dev, debug::Vb::Critical, "@todo Reimplement console");
}

}} // game::editor
} // clover
