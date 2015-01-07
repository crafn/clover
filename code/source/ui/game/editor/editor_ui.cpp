#include "editor_ui.hpp"
#include "game/editor/editor.hpp"
#include "resources/cache.hpp"
#include "ui/userinput.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

EditorUi::EditorUi(Editor& editor_):
	editor(&editor_),
	performanceTimer("Editor"){
	listenForEvent(global::Event::OnEditorViewCreate);
	listenForEvent(global::Event::OnEditorViewDestroy);

	print(debug::Ch::Dev, debug::Vb::Trivial, "EditorUi created");
}

EditorUi::~EditorUi(){
	print(debug::Ch::Dev, debug::Vb::Trivial, "EditorUi destroyed");
}

void EditorUi::onEvent(global::Event& e){
	performanceTimer.run();

	switch(e.getType()){
		case global::Event::OnEditorViewCreate:
			viewUis.pushBack( std::unique_ptr<EditorViewUi>(new EditorViewUi(*e(global::Event::Object).getPtr<EditorView>())) );
		break;

		case global::Event::OnEditorViewDestroy: {
			bool found= false;
			for (auto it = viewUis.begin(); it != viewUis.end(); ++it){
				if (&(*it)->getView() == e(global::Event::Object).getPtr<EditorView>()){
					viewUis.erase(it);
					found= true;
					break;
				}
			}
			ensure(found);

		}
		break;

		default:;
	}

	performanceTimer.pause();
}

void EditorUi::update(){
	performanceTimer.run();

	if (gUserInput->isTriggered(UserInput::DevSaveResources)){
		global::g_env.resCache->writeAllResources();
	}

	int32 toggled_ui= -1;
	for (uint32 i=0; i<viewUis.size(); ++i){
		if (gUserInput->isTriggered( UserInput::TriggerType(UserInput::ToggleEditorView_0+i) )){
			toggled_ui= (int32)i;
			break;
		}
	}

	if (toggled_ui >= 0){

		// Close others
		for (uint32 i=0; i<viewUis.size(); ++i){
			if (i != (uint32)toggled_ui)
				viewUis[i]->setActive(false);
		}

		viewUis[toggled_ui]->toggle();

	}

	for (auto& m : viewUis){
		if (m->isActive())
			m->update();
	}

	performanceTimer.pause();
}

bool EditorUi::hasActiveView(){
	for (uint32 i=0; i<viewUis.size(); ++i){
		if (viewUis[i]->isActive()){
			return true;
		}
	}

	return false;
}

}}} // ui::game::editor
} // clover