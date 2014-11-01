#ifndef CLOVER_HARDWARE_HID_DEVICE_MOUSE_HPP
#define CLOVER_HARDWARE_HID_DEVICE_MOUSE_HPP

#include "build.hpp"
#include "ui/hid/actionlistener.hpp"
#include "ui/hid/controls/control_bidirectional.hpp"
#include "ui/hid/controls/control_directional.hpp"
#include "ui/hid/controls/control_button.hpp"
#include "ui/hid/device.hpp"

namespace clover {
namespace hardware {

/// Representation of physical mouse device
class MouseHidDevice : public ui::hid::Device {
public:
	
	static Ptr create(const ConstructInfo& info){ return Ptr(new MouseHidDevice(info)); }
	virtual ~MouseHidDevice(){}

	virtual void update() override;
	virtual util::Str8 getTypeName() const override { return "Mouse"; }
	
protected:
	MouseHidDevice(const ConstructInfo& info);

private:
	static util::Vec2d rawToNormalized(util::Vec2d raw_pos);
	static void scrollCallback(GLFWwindow*, double x, double y);
	
	void pushLock();
	void popLock();
	void updateInputMode();
	
	// Reading through callbacks
	static util::Vec2d scroll;
	
	
	ui::hid::BiDirectionalControl& posControl;

	ui::hid::ButtonControl& lmbControl;
	ui::hid::ButtonControl& mmbControl;
	ui::hid::ButtonControl& rmbControl;
	
	ui::hid::BiDirectionalControl& scrollControl;
	
	util::Vec2d rawPosition;
	int32 moveCountAfterUnlock;
	
	/// Allows locking mouse position by actions
	util::LinkedList<ui::hid::ActionListener<>> pushLockListeners;
	util::LinkedList<ui::hid::ActionListener<>> popLockListeners;
	int32 locks;
	util::Vec2d rawLockPosition;
	
};

} // hardware
} // clover

#endif // CLOVER_HARDWARE_HID_DEVICE_MOUSE_HPP