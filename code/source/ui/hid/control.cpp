#include "control.hpp"
#include "device.hpp"

namespace clover {
namespace ui { namespace hid {

void Control::addChannel(const ContextChannelName& channel_name){
	additionalChannelNames.pushBack(channel_name);
}

Control::ContextChannelNames Control::getChannelNames() const {
	if (device)
		return util::duplicatesRemoved(device->getChannelNames().pushBacked(additionalChannelNames));
	else
		return additionalChannelNames;
}

}} // ui::hid
} // clover