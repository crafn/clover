#include "signaltypetraits.hpp"
#include "nodeevent.hpp"
#include "resources/exception.hpp"

namespace clover {
namespace nodes {

SignalType RuntimeSubSignalTypeTraits::signalType(SubSignalType S){

	#define SUBSIGNAL(x,n) \
		if (S == SubSignalType::x) return SubSignalTypeTraits<SubSignalType::x>::signalType;
	#include "subsignaltypes.def"
	#undef SUBSIGNAL

	throw global::Exception("RuntimeSubSignalTypeTraits::signalType(..): Invalid SubSignalType in %i", S);

}

util::Str8 RuntimeSubSignalTypeTraits::name(SubSignalType S){
	#define SUBSIGNAL(x,n) \
		if (S == SubSignalType::x) return SubSignalTypeTraits<SubSignalType::x>::name();
	#include "subsignaltypes.def"
	#undef SUBSIGNAL

	if (S == SubSignalType::None) return "";

	throw global::Exception("RuntimeSubSignalTypeTraits::name(..): Invalid SubSignalType in %i", S);
}

util::DynArray<SubSignalType> RuntimeSignalTypeTraits::subTypes(SignalType S){

	#define SIGNAL(x,n) \
		if (SignalType::x == S) \
			return (SignalTypeTraits<SignalType::x>::subTypes());
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::subTypes(..): Invalid SignalType: %i", S);
}

SignalValue RuntimeSignalTypeTraits::extract(SignalType S, SubSignalType extract_to, const util::Any& value){

	#define SIGNAL(x,n) \
		if (SignalType::x == S) return \
			SignalTypeTraits<SignalType::x>::extract(extract_to, util::anyCast<typename SignalTypeTraits<SignalType::x>::Value>(value));
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::extract(..): Invalid SignalType: %i", S);

}

void RuntimeSignalTypeTraits::combine(SignalType S, const SignalValue& value_from, SubSignalType type_to, util::Any value_to){

	#define SIGNAL(x,n) \
		if (SignalType::x == S){ \
			SignalTypeTraits<SignalType::x>::combine(value_from, type_to, *util::anyCast<SignalTypeTraits<SignalType::x>::Value*>(value_to)); \
			return; \
		}
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::combine(..): Invalid SignalType: %i", S);
}

util::Str8 RuntimeSignalTypeTraits::enumString(SignalType S){
	#define SIGNAL(x,n) \
		if (SignalType::x == S){ \
			return SignalTypeTraits<SignalType::x>::enumString(); \
		}
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::enumString(..): Invalid SignalType: %i", S);
}

SignalType RuntimeSignalTypeTraits::signalTypeFromEnumString(const util::Str8& enum_str){
	#define SIGNAL(x,n) \
		if (SignalTypeTraits<SignalType::x>::enumString() == enum_str){ \
			return SignalType::x; \
		}
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::signalTypeFromEnumString(..): Invalid enum string: %s", enum_str.cStr());
}

util::Any RuntimeSignalTypeTraits::defaultInitValue(SignalType S){
	#define SIGNAL(x,n) \
		if (SignalType::x == S){ \
			return SignalTypeTraits<SignalType::x>::defaultInitValue(); \
		}
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::defaultInitValue(..): Invalid SignalType: %i", S);
}

void RuntimeSignalTypeTraits::checkValue(SignalType S, const util::Any& value){
	#define SIGNAL(x,n) \
		if (SignalType::x == S){ \
			try { \
				util::anyCast<SignalTypeTraits<SignalType::x>::Value>(value); \
				return; \
			} \
			catch (...){ \
				throw resources::ResourceException("RuntimeSignalTypeTraits::checkValue(..): Invalid value type for SignalType %s", \
					RuntimeSignalTypeTraits::enumString(S).cStr()); \
			} \
		}
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("RuntimeSignalTypeTraits::checkValue(..): Invalid SignalType: %i", S);
}

util::DynArray<SignalType> RuntimeSignalTypeTraits::signalTypes(){
	util::DynArray<SignalType> ret;
	#define SIGNAL(x,n) \
		ret.pushBack(SignalType::x); 
	#include "signaltypes.def"
	#undef SIGNAL
	
	return (ret);
}

auto SignalTypeTraits<SignalType::Event>::defaultInitValue() -> Value{
	return Value();
}

} // nodes
} // clover