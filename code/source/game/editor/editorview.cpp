#include "editorview.hpp"
#include "editorcomponentfactory.hpp"
#include "global/event.hpp"

namespace clover {
namespace game { namespace editor {

EditorView::EditorView():
	active(false){
	global::Event e(global::Event::OnEditorViewCreate);
	e(global::Event::Object)= this;
	e.send();
}

EditorView::~EditorView(){
	global::Event e(global::Event::OnEditorViewDestroy);
	e(global::Event::Object)= this;
	e.send();
}

void EditorView::setActive(bool b){
	active= b;
}

EditorComponent& EditorView::createComponent(const util::Str8& name){
	components.pushBack(EditorComponentFactory::create(name));
	return *components.back().get();
}

void EditorView::destroyComponent(EditorComponent& comp){
	for (auto it= components.begin(); it != components.end(); ++it){
		if (&comp == it->get()){
			components.erase(it);
			return;
		}
	}
	
	ensure_msg(0, "Component not found");
}


bool EditorView::hasComponent(const util::Str8& type_name){
	for (const auto& comp : components){
		if (comp->getName() == type_name)
			return true;
	}
	return false;
}

}} // game::editor
} // clover