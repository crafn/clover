#ifndef CLOVER_UI_HID_DEVICE_HPP
#define CLOVER_UI_HID_DEVICE_HPP

#include "build.hpp"
#include "control.hpp"
#include "contextchannel.hpp"
#include "util/dyn_array.hpp"

#include <memory>

namespace clover {
namespace ui { namespace hid {

class Device {
public:
	using Ptr= std::shared_ptr<Device>;
	using ControlPtr= std::weak_ptr<Control>;
	using Name= util::Str8;
	using Uuid= util::Str8;

	struct ConstructInfo {
		Device::Name name;
		Device::Uuid uuid;
		ContextChannel::Names channelNames;
		bool allowTypicalControls;
	};

	Device(const ConstructInfo& info);
	virtual ~Device(){}

	/// Calls update and updates controls
	void deviceUpdate();

	virtual Name getTypeName() const = 0;
	const Name& getName() const { return name; }
	const Uuid& getUuid() const { return uuid; }
	const ContextChannel::Names& getChannelNames() const { return channelNames; }
	bool allowsTypicalControls() const { return allowTypicalControls; }

	/// Throws if control not found
	ControlPtr getControl(const Control::Name& name);

protected:
	virtual void update() = 0;

	template <typename T>
	T& addControl(const Control::Name& name){
		auto ptr= new T();
		ptr->setDevice(this);
		ptr->setName(name);
		controls.pushBack(Control::Ptr(ptr));
		return *ptr;
	}

private:
	Name name;
	Uuid uuid;
	ContextChannel::Names channelNames;
	bool allowTypicalControls;

	util::DynArray<Control::Ptr> controls;

};

}} // ui::hid
namespace util {

template <>
struct ObjectNodeTraits<ui::hid::Device::Ptr> {
	typedef ui::hid::Device::Ptr Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_UI_HID_DEVICE_HPP