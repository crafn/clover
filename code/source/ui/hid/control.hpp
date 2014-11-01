#ifndef CLOVER_UI_HID_CONTROL_HPP
#define CLOVER_UI_HID_CONTROL_HPP

#include "build.hpp"
#include "nodes/signaltypetraits.hpp"
#include "util/callbacker.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"

#include <memory>

namespace clover {
namespace ui { namespace hid {

class Device;
class VirtualControl;

struct ControlEvent {
	using Name= util::Str8;
	using ContextChannelNames= util::DynArray<util::Str8>; // To avoid circular dependency with ContextChannel::Names
	
	Name name;
	nodes::SignalValue value;
	ContextChannelNames channelNames;
};

struct OnControlEventCb : public util::SingleCallbacker<const ControlEvent&>{};

/// Base class for representation of a single control in a device (e.g. a button)
class Control : public util::Callbacker<OnControlEventCb> {
public:
	using Ptr= std::shared_ptr<Control>;
	using DevicePtr= Device*;
	using Name= util::Str8;
	using ContextChannelName= util::Str8; // To avoid circular dependency with ContextChannel::Names
	using ContextChannelNames= util::DynArray<ContextChannelName>;
	
	Control(): device(nullptr){}
	Control(const Control&)= delete;
	Control(Control&&)= delete;
	virtual ~Control(){}
	
	void addChannel(const ContextChannelName& channel_name);
	
	/// Trigger actions
	virtual void update() = 0;
	
	void setName(const Name& n){ name= n; }
	const Name& getName() const { return name; }
	
	DevicePtr getDevice() const { return device; }
	
	ContextChannelNames getChannelNames() const;
	
protected:
	friend class Device;
	void setDevice(DevicePtr dev){ device= dev; }
	
	void triggerEvent(const ControlEvent::Name& event_name){
		OnControlEventCb::trigger(ControlEvent{event_name, nodes::SignalValue(nodes::SignalType::Trigger, nodes::TriggerValue()), getChannelNames()});
	}
	
	template <nodes::SignalType S>
	void triggerEvent(const ControlEvent::Name& event_name, const typename nodes::SignalTypeTraits<S>::Value& value){
		OnControlEventCb::trigger(ControlEvent{event_name, nodes::SignalValue(S, value), getChannelNames()});
	}
		
private:
	DevicePtr device;
	Name name;
	ContextChannelNames additionalChannelNames;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_CONTROL_HPP