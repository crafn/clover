
template <SignalType S>
OutputSlot<S>::OutputSlot():
	BaseOutputSlot(S){
}

template <SignalType S>
OutputSlot<S>::~OutputSlot(){
}

template <SignalType S>
void OutputSlot<S>::send(const Value& value) const {
	try {
		for (auto& m : inputs){
			debug_ensure(m.slot);

			if (m.fromSub == SubSignalType::None && m.toSub == SubSignalType::None){
				// Normal routing between two slots

				debug_ensure(m.slot->getType() == S);
				
				sendImpl<S>(*m.slot, value); 
			}
			else {
				// SubSlot routing
				
				SignalValue v;
				
				if (m.fromSub != SubSignalType::None)
					v= std::move(RuntimeSignalTypeTraits::extract(type, m.fromSub, value)); // Extract component
				else
					v= std::move(SignalValue(type, value)); // From ordinary slot
				
				// Send to receiver slot
				
				if (m.toSub != SubSignalType::None){
					// Modify a certain component, requires last value of input slot
					
					#define SIGNAL(x,n) \
						if (SignalType::x == m.slot->getType()){ \
							auto value= SignalTypeTraits<SignalType::x>::defaultInitValue(); \
							value= getInputValue<SignalType::x>(*m.slot); \
							RuntimeSignalTypeTraits::combine(m.slot->getType(), v, m.toSub, &value); \
							sendImpl<SignalType::x>(*m.slot, value); \
						}
					#include "signaltypes.def"
					#undef SIGNAL
					
				}
				else {
					// Ordinary slot, just forward the value
					
					#define SIGNAL(x,n) \
						if (SignalType::x == m.slot->getType()) \
							sendImpl<SignalType::x>(*m.slot, v.get<SignalType::x>());
					#include "signaltypes.def"
					#undef SIGNAL

				}
					
			}
		}
	}	
	catch (const util::BadAnyCast& e){
		throw global::Exception("Bad any cast in OutputSlot::send(): %s", e.what());
	}
}

template <SignalType S>
void OutputSlot<S>::send(const util::Any& value) const {
	send(util::anyCast<Value>(value));
}

template <SignalType S>
template <SignalType T>
void OutputSlot<S>::sendImpl(BaseInputSlot& slot, const typename SignalTypeTraits<T>::Value& value) const {
	debug_ensure(slot.getType() == T);
	static_cast<InputSlot<T>*>(&slot)->receive(value);
}

template <SignalType S>
template <SignalType T>
const typename SignalTypeTraits<T>::Value& OutputSlot<S>::getInputValue(BaseInputSlot& slot) const {
	debug_ensure(slot.getType() == T);
	return static_cast<InputSlot<T>*>(&slot)->get();
}