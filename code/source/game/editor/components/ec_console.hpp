#ifndef CLOVER_GAME_EDITOR_EC_CONSOLE_HPP
#define CLOVER_GAME_EDITOR_EC_CONSOLE_HPP

#include "../editorcomponent.hpp"
#include "build.hpp"
#include "debug/print.hpp"

namespace clover {
namespace game { namespace editor {

class ConsoleEc;

template <>
struct EditorComponentTraits<ConsoleEc> {
	static const util::Str8 name(){ return "Console"; }
};

class ConsoleEc : public EditorComponent {
public:
	DECLARE_EDITORCOMPONENT(ConsoleEc);
	virtual ~ConsoleEc();
	
	void setVerbosity(debug::Vb v);
	debug::Vb getVerbosity() const;
	
	void setChannelActive(debug::Ch id, bool b);
	bool isChannelActive(debug::Ch id) const;
	
	void tryExecute(const util::Str8& string);

private:
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EC_CONSOLE_HPP
