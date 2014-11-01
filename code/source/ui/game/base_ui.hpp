#ifndef CLOVER_UI_GAME_BASE_UI_HPP
#define CLOVER_UI_GAME_BASE_UI_HPP

#include "audio/audioreceiver.hpp"
#include "build.hpp"
#include "game/worldentity_handle.hpp"
#include "global/eventreceiver.hpp"
#include "gui/element_layout_linear.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_we_icon.hpp"
#include "ingame_ui.hpp"
#include "ui/hid/taglistentry.hpp"
#include "visual/model_text.hpp"
#include "visual/camera.hpp"

namespace clover {
namespace gui {

class ButtonElement;
class Cursor;
class WorldInterfaceElement;

} // gui
namespace ui { namespace game { namespace editor {

class EditorUi;

}}} // ui::game::editor
namespace ui { namespace game {

/// Top of user interface logic
class BaseUi : public global::EventReceiver {
public:
    BaseUi();
	virtual ~BaseUi();
	
	virtual void onEvent(global::Event&);

    bool update();

	InGameUi* getInGameUi(){ return inGameUi.get(); }
	editor::EditorUi* getEditorUi(){ return editorUi.get(); }
	
protected:
	void enablePlayerInput();
	void disablePlayerInput();
	
	std::unique_ptr<InGameUi> inGameUi;
	std::unique_ptr<editor::EditorUi> editorUi;

    gui::Cursor& guiCursor;

    util::Vec2d cursorOnWorld;
	bool quit;
	
	ui::hid::ActionListener<> quitListener;
	ui::hid::TagListEntry devInputEntry;
	util::DynArray<ui::hid::TagListEntry> disablePlayersTagListEntries; 
};

extern BaseUi* gBaseUi;

}} // ui::game
} // clover

#endif // CLOVER_UI_GAME_BASE_UI_HPP´