#ifndef CLOVER_UI_EDITOR_EDITOR_UI_HPP
#define CLOVER_UI_EDITOR_EDITOR_UI_HPP

#include "build.hpp"
#include "common.hpp"
#include "global/eventreceiver.hpp"
#include "editorview_ui.hpp"

namespace clover {
namespace ui { namespace game { namespace editor {

class Editor;

class EditorUi : public global::EventReceiver {
public:
	EditorUi(Editor& editor);
	virtual ~EditorUi();

	virtual void onEvent(global::Event&);

	void update();
	
	bool hasActiveView();

private:
	Editor* editor;
	util::DynArray<std::unique_ptr<EditorViewUi>> viewUis;
	util::Timer performanceTimer;
};

}}} // ui::game::editor
} // clover

#endif // CLOVER_UI_EDITOR_EDITOR_UI_HPP