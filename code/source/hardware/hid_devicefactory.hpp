#ifndef CLOVER_HARDWARE_HID_DEVICE_FACTORY_HPP
#define CLOVER_HARDWARE_HID_DEVICE_FACTORY_HPP

#include "build.hpp"
#include "ui/hid/device.hpp"

namespace clover {
namespace hardware {

class HidDeviceFactory {
public:
	using Device= ui::hid::Device;

	static Device::Ptr createDevice(const Device::ConstructInfo& info);
	static bool isDeviceTypeName(const Device::Name& type_name);
};

} // hardware
} // clover

#endif // CLOVER_HARDWARE_HID_DEVICE_FACTORY_HPP