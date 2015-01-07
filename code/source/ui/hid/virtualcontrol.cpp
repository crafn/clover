#include "virtualcontrol.hpp"
#include "action.hpp"
#include "global/event.hpp"
#include "hardware/device.hpp"
#include "hardware/hid_devicefactory.hpp"
#include "ui/hid/hid_mgr.hpp"

namespace clover {
namespace ui { namespace hid {

VirtualControl::VirtualControl(const Action::Name& action_name, const util::DynArray<util::Str8>& splitted_source, ControlPtrs custom_controls)
		: actionName(action_name){
	HidMgr& hid_mgr= global::g_env.device->getHidMgr();
	
	if (splitted_source.size() < 3){
		util::Str8 str;
		for (const auto& m : splitted_source){
			str += m + ", ";
		}
		throw global::Exception("Invalid source value for hid control event: %s", str.cStr());
	}
	
	deviceString= splitted_source[0];
	controlName= splitted_source[1];
	controlEventName= splitted_source[2];
	
	if (custom_controls.empty()){
		// Search controls from devices
		HidMgr::DevicePtrs devices;
		
		if (hardware::HidDeviceFactory::isDeviceTypeName(deviceString)){
			// Device is marked by its type name (e.g. "Keyboard" instead of "keyboard0"), 
			// so create virtualcontrols for every device of that type which supports typical controls
			devices.pushBack(hid_mgr.getDevicesByTypeName(deviceString, true));
		}
		else {
			// A specific device name was used
			devices.pushBack(hid_mgr.getDevice(deviceString));
		}
		
		for (auto& device : devices){
			controls.pushBack(device.lock()->getControl(controlName));
		}
	}
	else {
		// Custom controls
		for (auto& custom_control : custom_controls)
			controls.pushBack(custom_control);
	}
		
		
	//print(debug::Ch::Device, debug::Vb::Trivial, "VirtualControl created: Action: %s, ControlEvent: %s", action_name.cStr(), controlEventName.cStr());
	
	// Listen every control
	
	for (auto& control : controls){
		ensure(!control.expired());
		
		controlListener.listen(*control.lock(), [this] (const ControlEvent& event){
			if (event.name != controlEventName)
				return;
			
			//print(debug::Ch::General, debug::Vb::Trivial, "Hid action: %s from event: %s, %s", actionName.cStr(), control.lock()->getName().cStr(), event.name.cStr());

			Action a(actionName, event.value);
			OnActionCb::trigger(a, *this, event.channelNames);
		});
	}
}

util::Str8 VirtualControl::getSourceString() const {
	return util::Str8::format("%s.%s.%s",
					deviceString.cStr(),
					controlName.cStr(),
					controlEventName.cStr());
}

}} // ui::hid
namespace util {

util::ObjectNode ObjectNodeTraits<ui::hid::VirtualControl::Ptr>::serialized(const Value& value){
	util::ObjectNode virtual_control_ob;
	virtual_control_ob[value->getActionName()].setValue(value->getSourceString());
	return (virtual_control_ob);
}

auto ObjectNodeTraits<ui::hid::VirtualControl::Ptr>::deserialized(const util::ObjectNode& ob, const ui::hid::Context& owner) -> Value {
	if (ob.size() != 1 || !ob.isObject())
		throw global::Exception("Invalid ui::hid::VirtualControl: %s", ob.generateText().cStr());
	
	// Format: "action_name " : "source_str"
	util::Str8 action_name= ob.getMemberNames()[0];
	util::Str8 source_str= ob.get(action_name).getValue<util::Str8>();
	
	util::DynArray<util::Str8> splitted_source= source_str.splitted('.');
	
	if (splitted_source.size() == 3 &&
		splitted_source[0] == "context" &&
		splitted_source[1] == "state"){
		// Special case: context state changes trigger actions
		return (ui::hid::VirtualControl::create(action_name, splitted_source, owner.getStateControls()));
	}
	
	return (ui::hid::VirtualControl::create(action_name, splitted_source));
}

} // util
} // clover