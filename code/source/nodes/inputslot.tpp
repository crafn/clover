
template <SignalType S>
InputSlot<S>::InputSlot()
		: BaseInputSlot(S)
		, value(std::move(SignalTypeTraits<S>::defaultInitValue())){
}

template <SignalType S>
InputSlot<S>::~InputSlot(){
}

template <SignalType S>
void InputSlot<S>::setDefaultValue(const util::Any& v){
	set(v);
}

template <SignalType S>
auto InputSlot<S>::get() const -> const Value& {
	return value;
}

template <SignalType S>
void InputSlot<S>::set(const util::Any& v){
	try {
		value= util::anyCast<typename SignalTypeTraits<S>::Value>(v);
	}
	catch(...){
		throw resources::ResourceException(util::Str8("Value is invalid type").cStr());
	}
}

template <SignalType S>
void InputSlot<S>::subSet(SubSignalType sub_from, const util::Any& sub_value, SubSignalType sub_to){
	SignalTypeTraits<S>::combine(SignalValue(sub_from, sub_value), sub_to, value);
}

template <SignalType S>
void InputSlot<S>::receive(const Value& value_){
	value= value_;
	setValueReceived();
}