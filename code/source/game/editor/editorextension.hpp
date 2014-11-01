#ifndef CLOVER_GAME_EDITOR_EDITOREXTENSION_HPP
#define CLOVER_GAME_EDITOR_EDITOREXTENSION_HPP

#include "build.hpp"

namespace clover {
namespace game { namespace editor {

class EditorExtension {
public:
	EditorExtension()= default;
	
	EditorExtension(const EditorExtension&)= delete;
	EditorExtension(EditorExtension&&)= delete;
	
	EditorExtension& operator=(const EditorExtension&)= delete;
	EditorExtension& operator==(EditorExtension&&)= delete;
	
	virtual ~EditorExtension()= default;
	
	virtual void update(){}
};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EDITOREXTENSION_HPP