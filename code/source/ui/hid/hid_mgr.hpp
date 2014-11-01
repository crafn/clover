#ifndef CLOVER_UI_HID_HID_MGR_HPP
#define CLOVER_UI_HID_HID_MGR_HPP

#include "action.hpp"
#include "build.hpp"
#include "context.hpp"
#include "device.hpp"

namespace clover {
namespace ui { namespace hid {
class Device;

/// Manages hid devices
class HidMgr {
public:
	
	using DevicePtr= std::weak_ptr<Device>;
	using DevicePtrs= util::DynArray<DevicePtr>;
	
	HidMgr();
	void create();
	virtual ~HidMgr(){}
	
	void update();
	
	/// Throws global::Exception if device is not found
	DevicePtr getDevice(const Device::Name& device_name) const;
	DevicePtrs getDevicesByTypeName(const Device::Name& device_type_name, bool only_which_allow_typical_controls= false) const;
	
	/// @return Every existing channel
	ContextChannel::Names getChannelNames() const;
	
private:
	void removeTagIndirection();
	util::DynArray<Context::Ptr> getContexts(const Context::Tags& tags);
	
	util::DynArray<Context::Ptr> contexts;
	util::DynArray<Device::Ptr> devices;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_HID_MGR_HPP