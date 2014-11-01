#include "hid_devicefactory.hpp"
#include "hid_device_gamepad.hpp"
#include "hid_device_keyboard.hpp"
#include "hid_device_mouse.hpp"

namespace clover {
namespace hardware {

ui::hid::Device::Ptr HidDeviceFactory::createDevice(const ui::hid::Device::ConstructInfo& info){
	/// @todo Support real uuids

	if (info.uuid == "defaultMouse")
		return MouseHidDevice::create(info);
	else if (info.uuid == "defaultKeyboard")
		return KeyboardHidDevice::create(info);
	else if (info.uuid == "defaultGamepad")
		return GamepadHidDevice::create(info);
		
	throw global::Exception("Unsupported device %s, %s", info.name.cStr(), info.uuid.cStr());
}

bool HidDeviceFactory::isDeviceTypeName(const Device::Name& type_name){
	if (type_name == "Mouse" || type_name == "Keyboard" || type_name == "Gamepad")
		return true;
	else
		return false;
}

} // hardware
} // clover
