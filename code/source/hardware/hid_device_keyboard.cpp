#include "hid_device_keyboard.hpp"
#include "hardware/device.hpp"

namespace clover {
namespace hardware {

util::DynArray<ui::hid::TextControl*> KeyboardHidDevice::textControls;
constexpr KeyboardHidDevice::KeyDef KeyboardHidDevice::keyList[];

KeyboardHidDevice::KeyboardHidDevice(const ConstructInfo& info)
	: Device(info)
	, textControl(addControl<ui::hid::TextControl>("text")){
	
	createButtonControls();
	
	textControls.pushBack(&textControl);
	
	glfwSetCharCallback(&global::g_env->device->getWindow(), characterCallback);
	glfwSetKeyCallback(&global::g_env->device->getWindow(), keyCallback);
}

KeyboardHidDevice::~KeyboardHidDevice(){
	textControls.remove(&textControl);
}

void KeyboardHidDevice::update(){
	ensure(keyControls.size() == keyCount);
	
	for (SizeType i= 0; i < keyCount; ++i){
		int32 key_state= glfwGetKey(&global::g_env->device->getWindow(), keyList[i].glfwCode);
		keyControls[i]->setValue(key_state == GLFW_PRESS);
	}
}

void KeyboardHidDevice::createButtonControls(){
	ensure(keyControls.empty());
	
	for (SizeType i= 0; i < keyCount; ++i){
		keyControls.pushBack(&addControl<ui::hid::ButtonControl>(keyList[i].name));
	}
}

void KeyboardHidDevice::characterCallback(GLFWwindow*, uint32 character){
	for (auto& text_control : textControls)
		text_control->append(character);
}

void KeyboardHidDevice::keyCallback(GLFWwindow* window, int32 key, int32 scancode, int32 action, int32 mod){
	if ((action == GLFW_PRESS || action == GLFW_REPEAT) && key == GLFW_KEY_BACKSPACE){
		for (auto& text_control : textControls)
			text_control->append(util::Str8::backspaceChar);
	}
}

} // hardware
} // clover