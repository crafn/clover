#include "editor.hpp"
#include "global/event.hpp"

namespace clover {
namespace game { namespace editor {

Editor::Editor(){
	global::Event e(global::Event::OnEditorCreate);
	e(global::Event::Object)= this;
	e.send();

	views.resize(viewCount);
}

Editor::~Editor(){
	views.clear();

	global::Event e(global::Event::OnEditorDestroy);
	e(global::Event::Object)= this;
	e.send();
}

void Editor::update(){
	extensionMgr.update();
}

}} // game::editor
} // clover