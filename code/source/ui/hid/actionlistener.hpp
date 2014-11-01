#ifndef CLOVER_UI_HID_ACTIONLISTENER_HPP
#define CLOVER_UI_HID_ACTIONLISTENER_HPP

#include "baseactionlistener.hpp"
#include "build.hpp"
#include "nodes/signaltypetraits.hpp"
#include "util/traits.hpp"
//.tpp
#include "global/event.hpp"
#include "hardware/device.hpp"

#include <functional>

namespace clover{
namespace ui { namespace hid {

/// Listens for certain action with value type of SignalType S.
/// Plain ActionListener<> is also supported if value is not needed.
/// Listening is stopped when destroyed or moved.
template <nodes::SignalType S= nodes::SignalType::None>
class ActionListener : public BaseActionListener {
public:
	using Base= BaseActionListener;
	using Value= typename nodes::SignalTypeTraits<S>::Value;
	using Callback= typename std::function<typename util::ToSignature<void, Value>::Type>;
	using GenericCb= std::function<void (nodes::SignalValue)>;

	ActionListener(	const ContextChannel::Name& channel_name,
					const Context::Tag& context_tag,
					const Action::Name& action_name,
					Callback on_action);
	ActionListener(	const ContextChannel::Name& channel_name,
					const Context::Tag& context_tag,
					const Action::Name& action_name,
					GenericCb on_action);
	ActionListener(const ActionListener& other);
	ActionListener(ActionListener&& other);
	virtual ~ActionListener();
	
	ActionListener& operator=(const ActionListener& other);
	ActionListener& operator=(ActionListener&& other);
	
	/// Triggers callback
	virtual void onAction(const Action& action) const { triggerCallback(action); }
	
private:
	static constexpr bool valueIsVoid= std::is_same<Value, void>::value;
	
	/// If Value != void
	template <bool C= !valueIsVoid>
	typename std::enable_if<C, GenericCb>::type
	createTypeSafeCallback(Callback cb) const {
		return createTypeSafeCallbackWithValue(cb);
	}
	
	/// If Value == void
	template <bool C= valueIsVoid, int Dummy= 0> // Extra parameter needed to prevent overloading error
	typename std::enable_if<C, GenericCb>::type
	createTypeSafeCallback(Callback cb) const {
		return createTypeSafeCallbackWithoutValue(cb);
	}

	template <int Dummy= 0>	
	GenericCb createTypeSafeCallbackWithValue(Callback on_action) const;
	template <int Dummy= 0>
	GenericCb createTypeSafeCallbackWithoutValue(Callback on_action) const;

	void triggerCallback(const Action& a) const;
	
	void startListening(const ContextChannel::Name& channel_name, const Context::Tag& context_tag, const Action::Name& action_name, GenericCb on_action);
	void startListening(const ActionListener& other);
	void stopListening();

	GenericCb onActionCallback;
};

#include "actionlistener.tpp"

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_ACTIONLISTENER_HPP