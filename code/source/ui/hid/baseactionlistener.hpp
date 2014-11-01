#ifndef CLOVER_UI_HID_BASEACTIONLISTENER_HPP
#define CLOVER_UI_HID_BASEACTIONLISTENER_HPP

#include "action.hpp"
#include "build.hpp"
#include "context.hpp"
#include "util/tuple.hpp"

namespace clover {
namespace ui { namespace hid {

/// Abstract base for ActionListener
class BaseActionListener {
public:
	using Listeners= util::DynArray<const BaseActionListener*>;
	using ListenersKey= util::Tuple<ContextChannel::Name, Context::Tag, Action::Name>;
	
	BaseActionListener();
	virtual ~BaseActionListener(){}
	
	const Context::Tag& getContextTag() const { return contextTag; }
	const Action::Name& getActionName() const { return actionName; }
	const ContextChannel::Name& getChannelName() const { return channelName; }
	
	/// Triggers callback
	virtual void onAction(const Action& action) const = 0;
	
	static const Listeners& getListeners(const ListenersKey& key){ return outsideActionListeners[key]; }
	
protected:
	/// Called when ActionListener starts/stops listening
	void onActionListeningStart(const BaseActionListener& listener);
	void onActionListeningStop(const BaseActionListener& listener);
	
	ListenersKey getListenerKey() const { return ListenersKey(channelName, contextTag, actionName); }
	
	bool isListening() const { return listening; }
	
	ContextChannel::Name channelName;
	Context::Tag contextTag;
	Action::Name actionName;
	
private:
	bool listening;
	
	/// Listeners from outside hid system listening actions
	using ListenerMap= util::HashMap<ListenersKey, Listeners>;
	static ListenerMap outsideActionListeners;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_BASEACTIONLISTENER_HPP