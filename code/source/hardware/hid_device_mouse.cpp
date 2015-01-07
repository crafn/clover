#include "hid_device_mouse.hpp"
#include "global/cfg_mgr.hpp"
#include "hardware/device.hpp"

#include <GLFW/glfw3.h>

namespace clover {
namespace hardware {

util::Vec2d MouseHidDevice::scroll;

MouseHidDevice::MouseHidDevice(const ConstructInfo& info)
		: Device(info)
		, posControl(addControl<ui::hid::BiDirectionalControl>("position"))
		, lmbControl(addControl<ui::hid::ButtonControl>("left"))
		, mmbControl(addControl<ui::hid::ButtonControl>("middle"))
		, rmbControl(addControl<ui::hid::ButtonControl>("right"))
		, scrollControl(addControl<ui::hid::BiDirectionalControl>("scroll"))
		, moveCountAfterUnlock(999)
		, locks(0){
	
	for (const auto& channel : getChannelNames()){
		pushLockListeners.pushBack(ui::hid::ActionListener<>(channel, "mouseDevice", "Mouse.position.pushLock", [this] (){ pushLock(); }));
		popLockListeners.pushBack(ui::hid::ActionListener<>(channel, "mouseDevice", "Mouse.position.popLock", [this] (){ popLock(); }));
	}
	
	glfwSetScrollCallback(&global::g_env.device->getWindow(), scrollCallback);

	updateInputMode();
}


void MouseHidDevice::update(){
	util::Vec2d last_raw_pos= rawPosition;
	glfwGetCursorPos(&global::g_env.device->getWindow(), &rawPosition.x, &rawPosition.y);
	
	if (locks == 0 && last_raw_pos != rawPosition)
		++moveCountAfterUnlock;
	
	// Just some arbitrary cap
	if (moveCountAfterUnlock > 100)
		moveCountAfterUnlock= 100;
	
	
	ensure(locks >= 0);
	if (locks == 0){
		rawLockPosition= rawPosition;
	}
	else {
		// Mouse position locked, don't allow movement
		glfwSetCursorPos(&global::g_env.device->getWindow(), rawLockPosition.x, rawLockPosition.y);
		// Two setValue calls are needed (another one is below if block), so
		// that deltas work but position remains ~same
		posControl.setValue(rawToNormalized(rawLockPosition));
	}
	
	// GLFW_CURSOR_DISABLED input mode during locking centers cursor position when 
	// changed back to normal mode (probably bug in glfw 3.0.1)
	// so don't update position until mouse is moved
	// (first move happens when glfw sets position, second when user moves)
	if (moveCountAfterUnlock > 1 || locks > 0)
		posControl.setValue(rawToNormalized(rawPosition));
		
	lmbControl.setValue(glfwGetMouseButton(&global::g_env.device->getWindow(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
	mmbControl.setValue(glfwGetMouseButton(&global::g_env.device->getWindow(), GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
	rmbControl.setValue(glfwGetMouseButton(&global::g_env.device->getWindow(), GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
	scrollControl.setValue(scroll);
}

util::Vec2d MouseHidDevice::rawToNormalized(util::Vec2d raw_pos){
	util::Vec2i screen_size= global::g_env.device->getViewportSize();
	return util::Vec2d{	(real64)raw_pos.x/(real64)screen_size.x*2.0 - 1.0,
					-(real64)raw_pos.y/(real64)screen_size.y*2.0 + 1.0};
}

void MouseHidDevice::scrollCallback(GLFWwindow*, double x, double y){
	scroll += util::Vec2d{x, y};
}

void MouseHidDevice::pushLock(){
	++locks;
	updateInputMode();
}

void MouseHidDevice::popLock(){
	--locks;
	util::Vec2d prev_raw_lock_position= rawLockPosition;
	updateInputMode();
	
	if (locks == 0)
		moveCountAfterUnlock= 0;
}

void MouseHidDevice::updateInputMode(){
	if (locks == 0){
		int32 input_mode= GLFW_CURSOR_HIDDEN;
		if (global::g_env.cfg->get<bool>("hardware::showCursor", true))
			input_mode= GLFW_CURSOR_NORMAL;
		
		glfwSetInputMode(&global::g_env.device->getWindow(), GLFW_CURSOR, input_mode);
	}
	else {
		// Prevent leaving from window when locked
		glfwSetInputMode(&global::g_env.device->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

} // hardware
} // clover