#include "base_ui.hpp"
#include "editor/editor_ui.hpp"
#include "ingame_ui.hpp"
#include "hardware/keyboard.hpp"
#include "game/basegamelogic.hpp"
#include "game/ingamelogic.hpp"
#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodes/native_instances/nodeinstance_cameratarget.hpp"
#include "visual/visual_mgr.hpp"
#include "visual/camera_mgr.hpp"
#include "ui/userinput.hpp"
#include "util/math.hpp"

namespace clover {
namespace ui { namespace game {

InGameUi::InGameUi(){
	audioRecv.assignNewReceiver();
	
	camera.select();
	camera.setPosition(util::Vec2d{0,10});
	camera.setScale(0.05);
}

void InGameUi::onEvent(global::Event& e){
	switch(e.getType()){
		default:;
	}
}

void InGameUi::update(){
	updateCamera();
	
	visual::Camera& camera= global::g_env->visualMgr->getCameraMgr().getSelectedCamera();
	audioRecv.setPosition(camera.getPosition());
}

util::DynArray<util::Str8> InGameUi::getPlayerStrings() const {
	/// @todo Remove this function
	return util::DynArray<util::Str8>{ "player0", "player1", "player2", "player3" };
}

void InGameUi::updateCamera(){
	real64 dt= util::limited(global::g_env->worldMgr->getDeltaTime(), 0.0, 0.05);

	if (gUserInput->isTriggered(UserInput::DevCamZoomIn)){
		camera.setTargetScale( camera.getScale()+0.2*camera.getScale() );
	}
	if (gUserInput->isTriggered(UserInput::DevCamZoomOut)){
		camera.setTargetScale( camera.getScale()-0.2*camera.getScale() );
	}
	
	/// @todo Remove hardcoded player entity behavior
	auto camera_target= nodes::CameraTargetNodeInstance::findTarget("player0");

	if (camera_target && !(global::g_env->ui->getEditorUi() && global::g_env->ui->getEditorUi()->hasActiveView())){
		camera.setTargetPosition(camera_target->getTransform().translation);
	}
	else {
		util::Vec2d camtarget= camera.getTargetPosition();
		
		real64 move_vel_mul= 5.0 / camera.getScale();
		
		if (gUserInput->isTriggered(UserInput::DevCamLeft)){
			camtarget += util::Vec2d{-1,0}*move_vel_mul*dt;
		}
		if (gUserInput->isTriggered(UserInput::DevCamRight)){
			camtarget += util::Vec2d{1,0}*move_vel_mul*dt;
		}
		if (gUserInput->isTriggered(UserInput::DevCamUp)){
			camtarget += util::Vec2d{0,1}*move_vel_mul*dt;
		}
		if (gUserInput->isTriggered(UserInput::DevCamDown)){
			camtarget += util::Vec2d{0,-1}*move_vel_mul*dt;
		}
		
		camera.setTargetPosition(camtarget);
	}
}

}} // ui::game
} // clover
