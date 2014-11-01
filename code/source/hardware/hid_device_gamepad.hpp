#ifndef CLOVER_HARDWARE_HID_DEVICE_GAMEPAD_HPP
#define CLOVER_HARDWARE_HID_DEVICE_GAMEPAD_HPP

#include "build.hpp"
#include "ui/hid/controls/control_bidirectional.hpp"
#include "ui/hid/controls/control_button.hpp"
#include "ui/hid/controls/control_directional.hpp"
#include "ui/hid/device.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace hardware {

class GamepadHidDevice : public ui::hid::Device {
public:
	static Ptr create(const ConstructInfo& info)
	{ return Ptr{new GamepadHidDevice{info}}; }
	
	virtual void update() override;
	virtual util::Str8 getTypeName() const override { return "Gamepad"; }

private:
	GamepadHidDevice(const ConstructInfo& info);

	int id;
	util::DynArray<ui::hid::ButtonControl*> buttons;
	util::DynArray<ui::hid::DirectionalControl*> axes;
	util::DynArray<ui::hid::BiDirectionalControl*> biAxes;
};

} // hardware
} // clover

#endif // CLOVER_HARDWARE_HID_DEVICE_GAMEPAD_HPP
