#ifndef CLOVER_UI_HID_INPUTCHANNEL_HPP
#define CLOVER_UI_HID_INPUTCHANNEL_HPP

#include "action.hpp"
#include "build.hpp"
#include "control.hpp"
#include "util/dyn_array.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"

namespace clover {
namespace ui { namespace hid {

class BaseActionListener;
class Context;

/// Receives actions from context and sends them to listeners
/// @note 	There are instances of ContextChannels for every context, because ContextChannel
///			handles context specific stuff (e.g. sequences). So when ActionListener listens
///			a specific channel, it actually listens several ContextChannels
class ContextChannel {
public:
	using Name= util::Str8;
	using Names= util::DynArray<util::Str8>;
	using ControlPtr= std::weak_ptr<Control>;

	ContextChannel(const Name& name, const Context& owner);

	void setEnabled(bool e= true);
	bool isEnabled() const { return enabled; }

	/// Filters, detects sequences and delivers actions to ActionListeners
	/// @todo return filtered action
	void onAction(const Action& a);

	ControlPtr getStateControl() const { return stateControl; }

private:
	const Context* owner;

	bool enabled;

	/// @todo Replace with a weak pointer to owner context
	Name name;

	/// Sometimes context change is needed to trigger actions
	/// Source: "context.state.enabled" / "context.state.disabled"
	Control::Ptr stateControl;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_INPUTCHANNEL_HPP