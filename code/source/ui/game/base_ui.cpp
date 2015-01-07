#include "base_ui.hpp"
#include "audio/audio_mgr.hpp"
#include "debug/draw.hpp"
#include "hardware/device.hpp"
#include "global/env.hpp"
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
#include "util/profiling.hpp"

namespace clover {
namespace ui { namespace game {

BaseUi::BaseUi()
	: guiCursor(global::g_env->guiMgr->getCursor())
	, quit(false)
	, quitListener("host", "emergency", "quit", [this] (){ quit= true; })
	, devInputEntry("host", "hostBaseUi"){ // Host channel should be the first to receive input
	if (!global::g_env->ui)
		global::g_env->ui= this;

	gUserInput= new UserInput;

	listenForEvent(global::Event::OnEditorCreate);
	listenForEvent(global::Event::OnEditorDestroy);

	inGameUi= std::unique_ptr<InGameUi>(new InGameUi());
}

BaseUi::~BaseUi(){
	if (global::g_env->ui == this)
		global::g_env->ui= nullptr;
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
	PROFILE();
	gUserInput->update();
	
	cursorOnWorld= hardware::gMouse->getPosition().converted(util::Coord::World).getValue();

	if (quit || glfwWindowShouldClose(&global::g_env->device->getWindow())){
		return false;
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
