#include "debug/print.hpp"
#include "hid_device_gamepad.hpp"

#include <GLFW/glfw3.h>

namespace clover {
namespace hardware {

GamepadHidDevice::GamepadHidDevice(const ConstructInfo& info)
	: Device(info)
	, id(GLFW_JOYSTICK_1) /// @todo Support multiple gamepads
{
	int raw_btn_count= 0;
	glfwGetJoystickButtons(id, &raw_btn_count);
	for (int i= 0; i < raw_btn_count; ++i) {
		util::Str8 name= util::Str8::format("button%i", (int32)buttons.size());
		buttons.pushBack(&addControl<ui::hid::ButtonControl>(name));
	}

	int raw_axis_count= 0;
	glfwGetJoystickAxes(id, &raw_axis_count); 
	for (int i= 0; i < raw_axis_count; ++i) {
		util::Str8 name= util::Str8::format("axis%i", i);
		axes.pushBack(&addControl<ui::hid::DirectionalControl>(name));

		if (i % 2 == 1) {
			util::Str8 name= util::Str8::format("axis%i%i", i-1, i);
			biAxes.pushBack(&addControl<ui::hid::BiDirectionalControl>(name));
		}
	}
}

void GamepadHidDevice::update()
{
	if (!glfwJoystickPresent(id))
		return;

	int raw_btn_count= 0;
	const unsigned char* raw_btns= NONULL(glfwGetJoystickButtons(id, &raw_btn_count));
	for (int i= 0; i < raw_btn_count && (int)buttons.size(); ++i) {
		buttons[i]->setValue(raw_btns[i] > 0);
	}

	int raw_axis_count= 0;
	const float* raw_axes= NONULL(glfwGetJoystickAxes(id, &raw_axis_count));
	for (int i= 0; i < raw_axis_count && i < (int)axes.size(); ++i) {
		axes[i]->setValue(raw_axes[i]);

		if (i % 2 == 1) {
			biAxes[i/2]->setValue(util::Vec2d{raw_axes[i-1], raw_axes[i]});
		}
	}
}

} // hardware
} // clover
