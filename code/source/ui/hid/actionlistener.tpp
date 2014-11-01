
template <nodes::SignalType S>
ActionListener<S>::ActionListener(	const ContextChannel::Name& channel_name,
									const Context::Tag& context_tag,
									const Action::Name& action_name,
									Callback on_action){
	startListening(channel_name, context_tag, action_name,
		createTypeSafeCallback(on_action));
}

template <nodes::SignalType S>
ActionListener<S>::ActionListener(	const ContextChannel::Name& channel_name,
									const Context::Tag& context_tag,
									const Action::Name& action_name,
									GenericCb on_action){
	startListening(channel_name, context_tag, action_name, on_action);
}

template <nodes::SignalType S>
ActionListener<S>::ActionListener(const ActionListener& other){
	startListening(other);
}

template <nodes::SignalType S>
ActionListener<S>::ActionListener(ActionListener&& other){
	other.stopListening();
	startListening(other);
}

template <nodes::SignalType S>
ActionListener<S>::~ActionListener(){
	stopListening();
}

template <nodes::SignalType S>
ActionListener<S>& ActionListener<S>::operator=(const ActionListener& other){
	stopListening();
	startListening(other);
	return *this;
}

template <nodes::SignalType S>
ActionListener<S>& ActionListener<S>::operator=(ActionListener&& other){
	other.stopListening();
	stopListening();
	startListening(other);
	return *this;
}

template <nodes::SignalType S>
template <int Dummy>
typename ActionListener<S>::GenericCb
ActionListener<S>::createTypeSafeCallbackWithValue(Callback on_action) const {
	return [on_action, this] (nodes::SignalValue sig_value){
		Value value;
		try {
			value= sig_value.get<S>();
		}
		catch (...){
			print(debug::Ch::General, debug::Vb::Critical,
				"ui::hid::ActionListener::onAction(..): Invalid type for action %s: %s for %s (fix control mappings)",
				getActionName().cStr(),
				nodes::RuntimeSignalTypeTraits::enumString(S).cStr(),
				nodes::RuntimeSignalTypeTraits::enumString(sig_value.getType()).cStr());
			return;
		}
		on_action(value);
	};
}

template <nodes::SignalType S>
template <int Dummy>
typename ActionListener<S>::GenericCb
ActionListener<S>::createTypeSafeCallbackWithoutValue(Callback on_action) const {
	return [on_action] (nodes::SignalValue){ on_action(); };
}

template <nodes::SignalType S>
void ActionListener<S>::triggerCallback(const Action& action) const {
	ensure(isListening());
	ensure(onActionCallback);
	
	onActionCallback(action.getValue());
}
	
template <nodes::SignalType S>
void ActionListener<S>::startListening(const ContextChannel::Name& channel_name, const Context::Tag& context_tag, const Action::Name& action_name, GenericCb on_action){
	channelName= channel_name;
	contextTag= context_tag;
	actionName= action_name;
	onActionCallback= on_action;
	
	Base::onActionListeningStart(*this);
}

template <nodes::SignalType S>
void ActionListener<S>::startListening(const ActionListener& other){
	startListening(other.channelName, other.contextTag, other.actionName, other.onActionCallback);
}

template <nodes::SignalType S>
void ActionListener<S>::stopListening(){
	if (isListening()){
		Base::onActionListeningStop(*this);
	}
}