#include "base_ui.hpp"
#include "audio/audio_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "hardware/device.hpp"
#include "global/event.hpp"
#include "gui/element_button.hpp"
#include "gui/element_layout_radial.hpp"
#include "gui/element_worldinterface.hpp"
#include "gui/gui_mgr.hpp"
#include "visual/visual_mgr.hpp"
#include "visual/entity_mgr.hpp"
#include "hardware/mouse.hpp"
#include "hardware/keyboard.hpp"
#include "game/save_mgr.hpp"
#include "game/world_mgr.hpp"
#include "game/worldchunk.hpp"
#include "game/worldentity_mgr.hpp"
#include "game/worldquery.hpp"
#include "game/basegamelogic.hpp"
#include "game/ingamelogic.hpp"
#include "ui/game/editor/editor_ui.hpp"
#include "game/editor/editor.hpp"
#include "ui/userinput.hpp"
#include "physics/phys_mgr.hpp"

namespace clover {
namespace ui { namespace game {

BaseUi* gBaseUi;

BaseUi::BaseUi()
	: guiCursor(gui::gGuiMgr->getCursor())
	, quit(false)
	, quitListener("host", "emergency", "quit", [this] (){ quit= true; })
	, devInputEntry("host", "hostBaseUi"){ // Host channel should be the first to receive input

	gUserInput= new UserInput;

	listenForEvent(global::Event::OnEditorCreate);
	listenForEvent(global::Event::OnEditorDestroy);

	inGameUi= std::unique_ptr<InGameUi>(new InGameUi());

}

BaseUi::~BaseUi(){
}

void BaseUi::onEvent(global::Event& e){
	switch(e.getType()){
		case global::Event::OnEditorCreate:
			ensure(!editorUi.get());
			editorUi= std::unique_ptr<editor::EditorUi>(
				new editor::EditorUi(*e(global::Event::Object).getPtr<game::editor::Editor>()));
		break;

		case global::Event::OnEditorDestroy:
			ensure(editorUi.get());
			editorUi.reset();
		break;

		default:;
	}
}

bool BaseUi::update(){
	gUserInput->update();
	
	cursorOnWorld= hardware::gMouse->getPosition().converted(util::Coord::World).getValue();

    //audioRecv2.setPosition(util::Vec2d(0));

    if (quit || glfwWindowShouldClose(&hardware::gDevice->getWindow())){
		return false;
	}

    if (hardware::gKeyboard->isPressed(GLFW_KEY_KP_0)){
        util::gGameClock->toggle();
    }

    if (hardware::gKeyboard->isDown(GLFW_KEY_KP_ADD)){
        util::gGameClock->setTimeScale(util::gGameClock->getTimeScale() + 0.2*util::gRealClock->getDeltaTime());
    }
	
    if (hardware::gKeyboard->isDown(GLFW_KEY_KP_SUBTRACT)){
        util::gGameClock->setTimeScale(util::gGameClock->getTimeScale() - 0.2*util::gRealClock->getDeltaTime());
    }

	if (editorUi){
		editorUi->update();

		if (editorUi->hasActiveView())
			disablePlayerInput();
		else
			enablePlayerInput();
	}

	if (inGameUi)
		inGameUi->update();

    return true;
}

void BaseUi::enablePlayerInput(){
	disablePlayersTagListEntries.clear();
}

void BaseUi::disablePlayerInput(){
	if (!disablePlayersTagListEntries.empty())
		return;

	for (const auto& channel : inGameUi->getPlayerStrings()){
		// Push "disable" -tag to player channels. Hopefully no context has it :p
		/// @todo Scripted player manager should take care of disabling player input
		disablePlayersTagListEntries.pushBack(ui::hid::TagListEntry(channel, "disable"));
	}
}

}} // ui::game
} // clover
