#include "gui/cursor.hpp"
#include "gui/gui_mgr.hpp"
#include "userinput.hpp"
#include "hardware/mouse.hpp"
#include "hardware/keyboard.hpp"
#include "util/ensure.hpp"

namespace clover {

UserInput *gUserInput= nullptr; 

constexpr UserInput::TriggerDef UserInput::triggerList[];

UserInput::UserInput():
	cursorLocked(0),
	keyTriggerLocked(0),
	writeListener("host", "gui", "newText", [this](const util::Str8& text){ writtenStr= text; }){
		
	for (SizeType i= 0; i < LastTriggerType; ++i){
		const TriggerDef& trigger_def= triggerList[i];
		
		ui::hid::ActionListener<> action_listener(trigger_def.channelName, trigger_def.contextName, trigger_def.actionName, [this, i](){ 
			triggerStates[i].actionTriggered= true;
		});
		
		actionListeners.pushBack(std::move(action_listener));
	}
	
}

void UserInput::update(){
	pos= gui::gGuiMgr->getCursor().getPosition().
		converted(util::Coord::View_Stretch).getValue();
	posChange= gui::gGuiMgr->getCursor().getDelta().
		converted(util::Coord::View_Stretch).getValue();
	
	for (int32 i=0; i<LastTriggerType; ++i){
		TriggerState& m= triggerStates[i];
		
		m.prevState= m.curState;
		m.curState= m.actionTriggered;
		
		m.actionTriggered= false;
	}
}

util::Coord UserInput::getCursorPosition(){
	return util::Coord::VSt(pos);
}


util::Coord UserInput::getCursorDifference(){
	return util::Coord::VSt(posChange);
}

void UserInput::pushCursorLock(){
	++cursorLocked;
	
	//hardware::gMouse->lock();
}

void UserInput::popCursorLock(){
	if (cursorLocked > 0)
		--cursorLocked;
	
	if (!cursorLocked){
		//hardware::gMouse->lock(false);
	}
}

void UserInput::pushLockForWriting(){
	++keyTriggerLocked;
}

void UserInput::popLockForWriting(){
	if (keyTriggerLocked > 0)
		--keyTriggerLocked;
}

util::Str8 UserInput::getWrittenString(){
	return writtenStr;
	return hardware::gKeyboard->getWrittenStr();
}

uint32 UserInput::getEraseCharCount(){
	return 0;
	return hardware::gKeyboard->getWrittenBackspaceCount();
}

bool UserInput::isTriggeredImpl(TriggerType t){
	if (keyTriggerLocked && (	t != GuiCancel && 
								t != GuiCause && 
								t != ToggleEditorView_0 &&
								t != ToggleEditorView_1 &&
								t != ToggleEditorView_2 &&
								t != ToggleEditorView_3 &&
								t != DevSaveResources &&
								t != GuiConfirm &&
								t != GuiPrevHistoryEntry &&
								t != GuiNextHistoryEntry)) return false;
	
	switch(t){
		case GuiCause: if (hardware::gMouse->isReleased(GLFW_MOUSE_BUTTON_LEFT)) return true; break;
		
		case GuiSecondaryCause: if (hardware::gMouse->isReleased(GLFW_MOUSE_BUTTON_RIGHT)) return true; break;
		
		case GuiHold: if (hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_LEFT)) return true; break;
		
		case GuiCancel: if (hardware::gMouse->isReleased(GLFW_MOUSE_BUTTON_RIGHT)) return true; break;
		
		case GuiStartLeftDragging: if (hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_LEFT)) return true; break;
		case GuiStartMiddleDragging: if (hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_MIDDLE)) return true; break;
		case GuiStartRightDragging: if (hardware::gMouse->isPressed(GLFW_MOUSE_BUTTON_RIGHT)) return true; break;
		
		case GuiStopDragging: if (	hardware::gMouse->isReleased(GLFW_MOUSE_BUTTON_LEFT) || 
									hardware::gMouse->isReleased(GLFW_MOUSE_BUTTON_MIDDLE) || 
									hardware::gMouse->isReleased(GLFW_MOUSE_BUTTON_RIGHT)) return true; break;
		
		case GuiConfirm: if (hardware::gKeyboard->isPressed(GLFW_KEY_ENTER)) return true; break;
		
		case GuiPrevHistoryEntry: if (hardware::gKeyboard->isPressed(GLFW_KEY_UP)) return true; break;
		case GuiNextHistoryEntry: if (hardware::gKeyboard->isPressed(GLFW_KEY_DOWN)) return true; break;
		
		case ToggleQuickSelectRing: if (hardware::gKeyboard->isPressed('Q')) return true; break;

		case ToggleInventory: if (hardware::gKeyboard->isPressed(GLFW_KEY_TAB)) return true; break;
		
		case NextLinkedQuickSelectEntity: if (hardware::gMouse->getWheelTurn() > 0 ) return true; break;
		
		case PrevLinkedQuickSelectEntity: if (hardware::gMouse->getWheelTurn() < 0) return true; break;
		
		case PrimaryAction:	if (hardware::gMouse->isDown(GLFW_MOUSE_BUTTON_LEFT)) return true; break;
		
		case ThrowAwayCharge: if (hardware::gKeyboard->isDown('G')) return true; break;
		
		case ThrowAwayLaunch: if (hardware::gKeyboard->isReleased('G')) return true; break;
		
		case RunLeft: if (hardware::gKeyboard->isDown('A')) return true; break;
		case RunRight: if (hardware::gKeyboard->isDown('D')) return true; break;
		case LaunchJump: if (hardware::gKeyboard->isPressed(GLFW_KEY_SPACE)) return true; break;
		case MaintainJump: if (hardware::gKeyboard->isDown(GLFW_KEY_SPACE)) return true; break;
		case Suicide: if (hardware::gKeyboard->isPressed('X')) return true; break;
		case Respawn: if (hardware::gKeyboard->isPressed('Z')) return true; break;
		
		case DevCamRight: if (hardware::gKeyboard->isDown('D')) return true; break;
		case DevCamUp: if (hardware::gKeyboard->isDown('W')) return true; break;
		case DevCamLeft: if (hardware::gKeyboard->isDown('A')) return true; break;
		case DevCamDown: if (hardware::gKeyboard->isDown('S')) return true; break;
		case DevCamZoomIn: if (hardware::gKeyboard->isDown('Y')) return true; break;
		case DevCamZoomOut: if (hardware::gKeyboard->isDown('H')) return true; break;
		
		case DevSaveResources: if (hardware::gKeyboard->isPressed(GLFW_KEY_F5)) return true; break;
		
		case ToggleEditorView_0: if (hardware::gKeyboard->isPressed(GLFW_KEY_F1)) return true; break;
		case ToggleEditorView_1: if (hardware::gKeyboard->isPressed(GLFW_KEY_F2)) return true; break;
		case ToggleEditorView_2: if (hardware::gKeyboard->isPressed(GLFW_KEY_F3)) return true; break;
		case ToggleEditorView_3: if (hardware::gKeyboard->isPressed(GLFW_KEY_F4)) return true; break;

		default: ensure_msg(0, "Odd TriggerType: %i", t);
	
	}
	return false;
}

} // clover
