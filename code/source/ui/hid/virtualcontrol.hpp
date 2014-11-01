#ifndef CLOVER_UI_HID_VIRTUALCONTROL_HPP
#define CLOVER_UI_HID_VIRTUALCONTROL_HPP

#include "action.hpp"
#include "build.hpp"
#include "contextchannel.hpp"
#include "control.hpp"
#include "util/cb_listener.hpp"

#include <memory>

namespace clover {
namespace ui { namespace hid {
class Context;
class VirtualControl;
struct OnActionCb : public util::SingleCallbacker<const Action&, const VirtualControl&, const ContextChannel::Names&>{};

/// VirtualControl translates a specific ControlEvent to Action
/// e.g. Action "Jump" is triggered when Keyboard0::KeyA::ButtonPress happens
class VirtualControl : public util::Callbacker<OnActionCb> {
public:
	using Ptr= std::shared_ptr<VirtualControl>;
	using ControlPtr= std::weak_ptr<Control>;
	using ControlPtrs= util::DynArray<ControlPtr>;
	
	/// Creates VirtualControl by searching devices and controls with splitted_source
	/// @param splitted_source {"device", "control", "event"}
	static Ptr create(const Action::Name& action_name, const util::DynArray<util::Str8>& splitted_source){
		return Ptr(new VirtualControl(action_name, splitted_source));
	}
	
	/// Creates VirtualControl with controls 'custom_controls' instead of searching by 'splitted_source'
	static Ptr create(const Action::Name& action_name, const util::DynArray<util::Str8>& splitted_source, ControlPtrs custom_controls){
		return Ptr(new VirtualControl(action_name, splitted_source, custom_controls));
	}
	
	VirtualControl(const VirtualControl&)= delete;
	VirtualControl(VirtualControl&&)= delete;
	virtual ~VirtualControl(){}
	
	const Action::Name& getActionName() const { return actionName; }
	util::Str8 getSourceString() const;

	ControlPtrs getControls() const { return controls; }
	
private:
	VirtualControl(const Action::Name& action_name, const util::DynArray<util::Str8>& splitted_source, ControlPtrs custom_controls= {});

	util::CbMultiListener<OnControlEventCb> controlListener;
	
	Action::Name actionName;
	util::Str8 deviceString;
	Control::Name controlName;
	ControlEvent::Name controlEventName;
	ControlPtrs controls;
};

}} // ui::hid
namespace util {

template <>
struct ObjectNodeTraits<ui::hid::VirtualControl::Ptr> {
	typedef ui::hid::VirtualControl::Ptr Value;
	static util::ObjectNode serialized(const Value& value);
	// Due to extra parameter, util::ObjectNode::getValue can't be used anymore
	static Value deserialized(const util::ObjectNode& ob_node, const ui::hid::Context& owner);
};

} // util
} // clover

#endif // CLOVER_UI_HID_VIRTUALCONTROL_HPP