template <typename Archive>
void RuntimeSignalTypeTraits::serializeValue(SignalType S, Archive& ar, const uint32 ver, util::Any& value){
		#define SIGNAL(x,n) \
			if (SignalType::x == S){ \
				/* If value.empty(): Put some rubbish (of the correct type) in the archive so that deserialization won't break */ \
				SignalTypeTraits<SignalType::x>::Value v= SignalTypeTraits<SignalType::x>::defaultInitValue(); \
				if (value.empty()) \
					value= v; \
				ar & util::anyCast<typename SignalTypeTraits<SignalType::x>::Value&>(value); \
				return; \
			}
		#include "signaltypes.def"
		#undef SIGNAL
		
	throw global::Exception("RuntimeSignalTypeTraits::serializeValue(..): Invalid SignalType: %i", S);
}
