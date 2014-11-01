#include "device.hpp"
#include "hardware/hid_devicefactory.hpp"

namespace clover {
namespace ui { namespace hid {

Device::Device(const ConstructInfo& info)
	: name(info.name)
	, uuid(info.uuid)
	, channelNames(info.channelNames)
	, allowTypicalControls(info.allowTypicalControls){
}

void Device::deviceUpdate(){
	update();
	
	for (auto& control : controls){
		control->update();
	}
}

Device::ControlPtr Device::getControl(const Control::Name& name){
	for (auto& control : controls){
		if (control->getName() == name)
			return ControlPtr(control);
	}
	throw global::Exception("ui::hid::Device::getControl(..): Control not found: %s", name.cStr());
}

}} // ui::hid
namespace util {

util::ObjectNode ObjectNodeTraits<ui::hid::Device::Ptr>::serialized(const Value& value){
	util::ObjectNode ob;
	ob["name"].setValue(value->getName());
	ob["uuid"].setValue(value->getUuid());
	ob["channels"].setValue(value->getChannelNames());
	ob["allowTypicalControls"].setValue(value->allowsTypicalControls());
	return ob;
}

auto ObjectNodeTraits<ui::hid::Device::Ptr>::deserialized(const util::ObjectNode& ob_node) -> Value {
	ui::hid::Device::ConstructInfo info;
	
	info.name= ob_node.get("name").getValue<ui::hid::Device::Name>();
	info.uuid= ob_node.get("uuid").getValue<ui::hid::Device::Uuid>();
	info.channelNames= ob_node.get("channels").getValue<ui::hid::ContextChannel::Names>();
	info.allowTypicalControls= ob_node.get("allowTypicalControls").getValue<bool>();
	
	return hardware::HidDeviceFactory::createDevice(info);
}

} // util
} // clover