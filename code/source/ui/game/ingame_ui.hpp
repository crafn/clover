#ifndef CLOVER_UI_GAME_INGAME_UI_HPP
#define CLOVER_UI_GAME_INGAME_UI_HPP

#include "audio/audioreceiverhandle.hpp"
#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "gui/element_worldinterface.hpp"
#include "visual/camera.hpp"

namespace clover {
namespace ui { namespace game {

/// Handles all ingame-related user interface stuff
class InGameUi : public global::EventReceiver {
public:
	InGameUi();

	virtual void onEvent(global::Event&);
	
	void update();
	
	gui::WorldInterfaceElement& getWorldElement(){ return worldElement; }
	util::DynArray<util::Str8> getPlayerStrings() const;

private:
	void updateCamera();
	
	gui::WorldInterfaceElement worldElement;
	visual::Camera camera;
    audio::AudioReceiverHandle audioRecv;
};

}} // ui::game
} // clover

#endif // CLOVER_UI_GAME_INGAME_UI_HPP
