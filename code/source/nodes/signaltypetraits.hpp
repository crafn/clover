#ifndef CLOVER_NODES_SIGNALTYPETRAITS_HPP
#define CLOVER_NODES_SIGNALTYPETRAITS_HPP

#include "animation/armaturepose.hpp"
#include "build.hpp"
#include "collision/shape.hpp"
#include "game/worldentity_handle.hpp"
#include "resources/resource_ref.hpp"
#include "script/typestring.hpp"
#include "signaltype.hpp"
#include "util/any.hpp"
#include "util/dyn_array.hpp"
#include "util/quaternion.hpp"
#include "util/traits.hpp"
#include "util/transform.hpp"
#include "util/vector.hpp"

/// @todo Replace with util::Any
#include <boost/any.hpp>

namespace clover {
namespace nodes {

template <SignalType>
struct SignalTypeTraits;

template <SubSignalType>
struct SubSignalTypeTraits;


/// Determines SubSignals' traits stuff at runtime
struct RuntimeSubSignalTypeTraits {
	static SignalType signalType(SubSignalType S);
	static util::Str8 name(SubSignalType S);
};

/// Used for subsignalling
class SignalValue {
public:
	SignalValue(){ set(SignalType::None, 0); }
	SignalValue(SignalType t, const util::Any& v){ set(t, v); }
	SignalValue(SubSignalType t, const util::Any& v){ set(RuntimeSubSignalTypeTraits::signalType(t), v); }
	void set(SignalType t, const util::Any& v){ type= t; value= v; }

	template <SignalType S>
	const typename SignalTypeTraits<S>::Value& get() const {
		ensure(S == type);
		return util::anyCast<const typename SignalTypeTraits<S>::Value&>(value);
	}

	template <SubSignalType S>
	const typename SignalTypeTraits<SubSignalTypeTraits<S>::signalType>::Value& get() const {
		return get<SubSignalTypeTraits<S>::signalType>();
	}

	util::Any getAny() const { return value; }

	SignalType getType() const { return type; }

private:
	SignalType type;
	util::Any value;
};



/// Determines Signals' traits stuff at runtime
struct RuntimeSignalTypeTraits {
	/// Possible SubTypes
	static util::DynArray<SubSignalType> subTypes(SignalType S);

	static SignalValue extract(SignalType S, SubSignalType extract_to, const util::Any& value_from);

	/// value_to must be a pointer!
	static void combine(SignalType S, const SignalValue& value_from, SubSignalType type_to, util::Any value_to);

	static util::Str8 enumString(SignalType S);
	static SignalType signalTypeFromEnumString(const util::Str8& enum_str);

	// Input's value if no init value is set for CompositionNodeSlot, which can happen with dynamic slots
	static util::Any defaultInitValue(SignalType S);

	/// Used to serialize and deserialize a signal value
	/// Sets undefined data to "value" if value.empty() is/was true (also in serialization that is)
	template <typename Archive>
	static void serializeValue(SignalType S, Archive& ar, const uint32 ver, util::Any& value);

	/// Throws resources::ResourceException if util::Any not correct type
	static void checkValue(SignalType S, const util::Any& value);

	/// Returns all possible SignalTypes
	static util::DynArray<SignalType> signalTypes();
};



// SubSignalTypeTraits

template <> struct SubSignalTypeTraits<SubSignalType::X> {
	static constexpr SignalType signalType= SignalType::Real; // Corresponding signal by value type
	static util::Str8 name(){ return "X"; } // Name in GUI
};

template <> struct SubSignalTypeTraits<SubSignalType::Y> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "Y"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Z> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "Z"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Translation2> {
	static constexpr SignalType signalType= SignalType::Vec2;
	static util::Str8 name(){ return "Translation2"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Translation3> {
	static constexpr SignalType signalType= SignalType::Vec3;
	static util::Str8 name(){ return "Translation3"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::RotationZ> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "RotationZ"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::RotationQ> {
	static constexpr SignalType signalType= SignalType::Quaternion;
	static util::Str8 name(){ return "RotationQ"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::ScaleX> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "ScaleX"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::ScaleY> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "ScaleY"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::ScaleZ> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "ScaleZ"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Scale2> {
	static constexpr SignalType signalType= SignalType::Vec2;
	static util::Str8 name(){ return "Scale2"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Scale3> {
	static constexpr SignalType signalType= SignalType::Vec3;
	static util::Str8 name(){ return "Scale3"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::RtTransform2> {
	static constexpr SignalType signalType= SignalType::RtTransform2;
	static util::Str8 name(){ return "RtTransform2"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::SrtTransform2> {
	static constexpr SignalType signalType= SignalType::SrtTransform2;
	static util::Str8 name(){ return "SrtTransform2"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::SrtTransform3> {
	static constexpr SignalType signalType= SignalType::SrtTransform3;
	static util::Str8 name(){ return "SrtTransform3"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Trigger> {
	static constexpr SignalType signalType= SignalType::Trigger;
	static util::Str8 name(){ return "Trigger"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::Resource> {
	static constexpr SignalType signalType= SignalType::String;
	static util::Str8 name(){ return "Resource"; }
};

template <> struct SubSignalTypeTraits<SubSignalType::W> {
	static constexpr SignalType signalType= SignalType::Real;
	static util::Str8 name(){ return "W"; }
};


/// Empty value type for SignalType::Trigger
struct TriggerValue {
	template <typename Archive>
	void serialize(Archive& ar, uint32 version){}

	TriggerValue(){}
	TriggerValue& operator=(const TriggerValue&){ return *this; }
};

// SignalTypeTraits

#define DEFAULT_EXTRACT(signal_type, subsignal_type) \
	case SubSignalType::Trigger: return SignalValue(SubSignalType::Trigger, TriggerValue()); break; \
	default: throw global::Exception("SignalTypeTraits<SignalType::%s>::extract(..): SubSignalType not supported: %i", #signal_type, subsignal_type);

#define DEFAULT_COMBINE(signal_type, subsignal_type) \
	case SubSignalType::Trigger: break; \
	default: throw global::Exception("SignalTypeTraits<SignalType::%s>::combine(..): SubSignalType not supported: %i", #signal_type, subsignal_type);

template <>
struct SignalTypeTraits<SignalType::None> {
	typedef void Value;
	static util::Str8 enumString(){ return "None"; }
};

template <>
struct SignalTypeTraits<SignalType::Real> {
	typedef real64 Value;
	static util::Str8 enumString(){ return "Real"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& v){
		switch(type){
			DEFAULT_EXTRACT(Real, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(Real, type)
		}
	}

	static Value defaultInitValue(){ return Value(0.0); }

};

template <>
struct SignalTypeTraits<SignalType::Integer> {
	typedef int64 Value;
	static util::Str8 enumString(){ return "Integer"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& v){
		switch(type){
			DEFAULT_EXTRACT(Integer, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(Integer, type)
		}
	}

	static Value defaultInitValue(){ return Value(0); }
};

template <>
struct SignalTypeTraits<SignalType::String> {
	typedef util::Str8 Value;
	static util::Str8 enumString(){ return "String"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& v){
		switch(type){
			DEFAULT_EXTRACT(String, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(String, type)
		}
	}

	static Value defaultInitValue(){ return Value(""); }

};

template <>
struct SignalTypeTraits<SignalType::Boolean> {
	typedef bool Value;
	static util::Str8 enumString(){ return "Boolean"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& v){
		switch(type){
			DEFAULT_EXTRACT(Boolean, type);
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(Boolean, type)
		}
	}

	static Value defaultInitValue(){ return Value(false); }
};

template <>
struct SignalTypeTraits<SignalType::Vec2> {
	typedef util::Vec2d Value;
	static util::Str8 enumString(){ return "Vec2"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {	SubSignalType::X,
					SubSignalType::Y,
					SubSignalType::Trigger};
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::X: return SignalValue(type, from.x);
			case SubSignalType::Y: return SignalValue(type, from.y);
			DEFAULT_EXTRACT(util::Vec2, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			case SubSignalType::X: to.x= from.get<SubSignalType::X>(); return;
			case SubSignalType::Y: to.y= from.get<SubSignalType::Y>(); return;
			DEFAULT_COMBINE(util::Vec2, type)
		}
	}

	static Value defaultInitValue(){ return Value(0); }
};

template <>
struct SignalTypeTraits<SignalType::Vec3> {
	typedef util::Vec3d Value;
	static util::Str8 enumString(){ return "Vec3"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {	SubSignalType::X,
					SubSignalType::Y,
					SubSignalType::Z,
					SubSignalType::Trigger};
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::X: return SignalValue(type, from.x);
			case SubSignalType::Y: return SignalValue(type, from.y);
			case SubSignalType::Z: return SignalValue(type, from.z);
			DEFAULT_EXTRACT(util::Vec3, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			case SubSignalType::X: to.x= from.get<SubSignalType::X>(); return;
			case SubSignalType::Y: to.y= from.get<SubSignalType::Y>(); return;
			case SubSignalType::Z: to.z= from.get<SubSignalType::Z>(); return;
			DEFAULT_COMBINE(util::Vec3, type)
		}
	}

	static Value defaultInitValue(){ return Value(0); }
};

template <>
struct SignalTypeTraits<SignalType::Vec4> {
	typedef util::Vec4d Value;
	static util::Str8 enumString(){ return "Vec4"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {	SubSignalType::X,
					SubSignalType::Y,
					SubSignalType::Z,
					SubSignalType::W,
					SubSignalType::Trigger};
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::X: return SignalValue(type, from.x);
			case SubSignalType::Y: return SignalValue(type, from.y);
			case SubSignalType::Z: return SignalValue(type, from.z);
			case SubSignalType::W: return SignalValue(type, from.w);
			DEFAULT_EXTRACT(util::Vec4, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			case SubSignalType::X: to.x= from.get<SubSignalType::X>(); return;
			case SubSignalType::Y: to.y= from.get<SubSignalType::Y>(); return;
			case SubSignalType::Z: to.z= from.get<SubSignalType::Z>(); return;
			case SubSignalType::W: to.w= from.get<SubSignalType::W>(); return;
			DEFAULT_COMBINE(util::Vec4, type)
		}
	}

	static Value defaultInitValue(){ return Value(0); }

};

template <>
struct SignalTypeTraits<SignalType::RtTransform2> {
	typedef util::RtTransform2d Value;
	static util::Str8 enumString(){ return "RtTransform2"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {	SubSignalType::X,
					SubSignalType::Y,
					SubSignalType::Translation2,
					SubSignalType::RotationZ,
					SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::X: return SignalValue(type, from.translation.x);
			case SubSignalType::Y: return SignalValue(type, from.translation.y);
			case SubSignalType::Translation2: return SignalValue(type, from.translation);
			case SubSignalType::RotationZ: return SignalValue(type, from.rotation);
			DEFAULT_EXTRACT(RtTransform2, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			case SubSignalType::X: to.translation.x= from.get<SubSignalType::X>(); return;
			case SubSignalType::Y: to.translation.y= from.get<SubSignalType::Y>(); return;
			case SubSignalType::Translation2: to.translation= from.get<SubSignalType::Translation2>(); return;
			case SubSignalType::RotationZ: to.rotation= from.get<SubSignalType::RotationZ>(); return;
			DEFAULT_COMBINE(RtTransform2, type)
		}
	}

	static Value defaultInitValue(){ return Value(0.0, {0,0}); }

};

template <>
struct SignalTypeTraits<SignalType::Quaternion> {
	typedef util::Quatd Value;
	static util::Str8 enumString(){ return "Quaternion"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			DEFAULT_EXTRACT(Quaternion, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(Quaternion, type)
		}
	}

	static Value defaultInitValue(){ return Value(); }

};

template <>
struct SignalTypeTraits<SignalType::SrtTransform2> {
	typedef util::SrtTransform2d Value;
	static util::Str8 enumString(){ return "SrtTransform2"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {	SubSignalType::X,
					SubSignalType::Y,
					SubSignalType::Translation2,
					SubSignalType::RotationZ,
					SubSignalType::ScaleX,
					SubSignalType::ScaleY,
					SubSignalType::Scale2,
					SubSignalType::RtTransform2,
					SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::Translation2: return SignalValue(type, from.translation);
			case SubSignalType::X: return SignalValue(type, from.translation.x);
			case SubSignalType::Y: return SignalValue(type, from.translation.y);
			case SubSignalType::RotationZ: return SignalValue(type, from.rotation);
			case SubSignalType::ScaleX: return SignalValue(type, from.scale.x);
			case SubSignalType::ScaleY: return SignalValue(type, from.scale.y);
			case SubSignalType::Scale2: return SignalValue(type, from.scale);
			case SubSignalType::RtTransform2: return SignalValue(type, from.rtTransform());
			DEFAULT_EXTRACT(SrtTransform2, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){

			case SubSignalType::X: to.translation.x= from.get<SubSignalType::X>(); return;
			case SubSignalType::Y: to.translation.y= from.get<SubSignalType::Y>(); return;
			case SubSignalType::Translation2: to.translation= from.get<SubSignalType::Translation2>(); return;
			case SubSignalType::RotationZ: to.rotation= from.get<SubSignalType::RotationZ>(); return;
			case SubSignalType::ScaleX: to.scale.x= from.get<SubSignalType::ScaleX>(); return;
			case SubSignalType::ScaleY: to.scale.y= from.get<SubSignalType::ScaleY>(); return;
			case SubSignalType::Scale2: to.scale= from.get<SubSignalType::Scale2>(); return;
			case SubSignalType::RtTransform2: to= commonReplaced(to, from.get<SubSignalType::RtTransform2>()); return;
			DEFAULT_COMBINE(SrtTransform2, type)
		}
	}

	static Value defaultInitValue(){ return Value({1,1}, 0, {0,0}); }
};

template <>
struct SignalTypeTraits<SignalType::SrtTransform3> {
	typedef util::SrtTransform3d Value;
	static util::Str8 enumString(){ return "SrtTransform3"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {	SubSignalType::X,
					SubSignalType::Y,
					SubSignalType::Z,
					SubSignalType::Translation2,
					SubSignalType::Translation3,
					SubSignalType::RotationZ,
					SubSignalType::RotationQ,
					SubSignalType::ScaleX,
					SubSignalType::ScaleY,
					SubSignalType::ScaleZ,
					SubSignalType::Scale2,
					SubSignalType::Scale3,
					SubSignalType::RtTransform2,
					SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::X: return SignalValue(type, from.translation.x);
			case SubSignalType::Y: return SignalValue(type, from.translation.y);
			case SubSignalType::Z: return SignalValue(type, from.translation.z);
			case SubSignalType::Translation2: return SignalValue(type, from.translation.xy());
			case SubSignalType::Translation3: return SignalValue(type, from.translation);
			case SubSignalType::RotationZ: return SignalValue(type, from.rotation.rotationZ());
			case SubSignalType::RotationQ: return SignalValue(type, from.rotation);
			case SubSignalType::ScaleX: return SignalValue(type, from.scale.x);
			case SubSignalType::ScaleY: return SignalValue(type, from.scale.y);
			case SubSignalType::ScaleZ: return SignalValue(type, from.scale.z);
			case SubSignalType::Scale2: return SignalValue(type, from.scale.xy());
			case SubSignalType::Scale3: return SignalValue(type, from.scale);
			case SubSignalType::RtTransform2: return SignalValue(type, commonReplaced(util::RtTransform2d{}, from));
			DEFAULT_EXTRACT(SrtTransform3, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			case SubSignalType::X: to.translation.x= from.get<SubSignalType::X>(); return;
			case SubSignalType::Y: to.translation.y= from.get<SubSignalType::Y>(); return;
			case SubSignalType::Z: to.translation.z= from.get<SubSignalType::Z>(); return;
			case SubSignalType::Translation2: to.translation= commonReplaced(to.translation, from.get<SubSignalType::Translation2>()); return;
			case SubSignalType::Translation3: to.translation= from.get<SubSignalType::Translation3>(); return;
			case SubSignalType::RotationZ: to.rotation= Value::Rotation::byRotationAxis({0.0, 0.0, 1.0}, from.get<SubSignalType::RotationZ>()); return;
			case SubSignalType::RotationQ: to.rotation= from.get<SubSignalType::RotationQ>(); return;
			case SubSignalType::ScaleX: to.scale.x= from.get<SubSignalType::ScaleX>(); return;
			case SubSignalType::ScaleY: to.scale.y= from.get<SubSignalType::ScaleY>(); return;
			case SubSignalType::ScaleZ: to.scale.z= from.get<SubSignalType::ScaleZ>(); return;
			case SubSignalType::Scale2: to.scale= commonReplaced(to.scale, from.get<SubSignalType::Scale2>()); return;
			case SubSignalType::Scale3: to.scale= from.get<SubSignalType::Scale3>(); return;
			case SubSignalType::RtTransform2: to= commonReplaced(to, from.get<SubSignalType::RtTransform2>()); return;
			DEFAULT_COMBINE(SrtTransform3, type)
		}
	}

	static Value defaultInitValue(){ return Value({1, 1, 1}, Value::Rotation(), {0, 0, 0}); }
};

class NodeEvent;

template <>
struct SignalTypeTraits<SignalType::Event> {
	typedef NodeEvent Value;
	static util::Str8 enumString(){ return "Event"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { SubSignalType::Trigger };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			DEFAULT_EXTRACT(global::Event, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(global::Event, type)
		}
	}

	static Value defaultInitValue();

};

template <>
struct SignalTypeTraits<SignalType::EventType> {
	typedef util::Str8 Value;
	static util::Str8 enumString(){ return "EventType"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {};
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			DEFAULT_EXTRACT(global::Event, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(global::Event, type)
		}
	}

	static Value defaultInitValue(){ return util::Str8(""); }

};

template <>
struct SignalTypeTraits<SignalType::Trigger> {
	typedef TriggerValue Value;
	static util::Str8 enumString(){ return "Trigger"; }

	static util::DynArray<SubSignalType> subTypes(){
		return {};
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			DEFAULT_EXTRACT(global::Event, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(global::Event, type)
		}
	}

	static Value defaultInitValue(){ return Value(); }
};

template <>
struct SignalTypeTraits<SignalType::EventArray> {
	typedef util::DynArray<SignalTypeTraits<SignalType::Event>::Value> Value;
	static util::Str8 enumString(){ return "EventArray"; }

	static util::DynArray<SubSignalType> subTypes(){

		return { /* Support for separate input and output subtypes needed */ };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			DEFAULT_EXTRACT(EventArray, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			DEFAULT_COMBINE(EventArray, type)
		}
	}

	static Value defaultInitValue(){ return Value(); }
};

#define DEFAULT_SIGNALTYPETRAITS(enum_name, type_name)								\
template <>																			\
struct SignalTypeTraits<SignalType::enum_name> {									\
	typedef type_name Value;														\
	static util::Str8 enumString(){ return #enum_name; }								  \
																					\
	static util::DynArray<SubSignalType> subTypes(){									  \
		return {};																	\
	}																				\
																					\
	static SignalValue extract(SubSignalType type, const Value& from){				\
		switch(type){																\
			DEFAULT_EXTRACT(enum_name, type)										\
		}																			\
	}																				\
																					\
	static void combine(const SignalValue& from, SubSignalType type, Value& to){	\
		switch(type){																\
			DEFAULT_COMBINE(enum_name, type)										\
		}																			\
	}																				\
																					\
	static Value defaultInitValue(){ return Value(); }								\
																					\
};

DEFAULT_SIGNALTYPETRAITS(ArmaturePose, animation::ArmaturePose)

template <>
struct SignalTypeTraits<SignalType::Shape> {
	typedef resources::ResourceRef<collision::Shape> Value;
	static util::Str8 enumString(){ return "Shape"; }

	static util::DynArray<SubSignalType> subTypes(){
		return { SubSignalType::Resource };
	}

	static SignalValue extract(SubSignalType type, const Value& from){
		switch(type){
			case SubSignalType::Resource: return SignalValue(type, from.get().getIdentifier());
			DEFAULT_EXTRACT(Shape, type)
		}
	}

	static void combine(const SignalValue& from, SubSignalType type, Value& to){
		switch(type){
			case SubSignalType::Resource: to= Value(from.get<SubSignalType::Resource>()); return;
			DEFAULT_COMBINE(Shape, type)
		}
	}

	static Value defaultInitValue(){ return Value(); }

};

DEFAULT_SIGNALTYPETRAITS(WeHandle, game::WeHandle)

#undef DEFAULT_SIGNALTYPETRAITS
#undef DEFAULT_EXTRACT
#undef DEFAULT_COMBINE

template <template <typename> class T>
void callForEverySignalValueType(){
	#define SIGNAL(x, n) T<SignalTypeTraits<SignalType::x>::Value>::invoke();
	#include "signaltypes.def"
	#undef SIGNAL
}

#include "signaltypetraits.tpp"

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::TriggerValue> {
	static util::Str8 type(){ return "::TriggerValue"; }
};

template <>
struct TypeStringTraits<nodes::SignalType> {
	static util::Str8 type(){ return "::SignalType"; }
};

template <>
struct TypeStringTraits<nodes::SubSignalType> {
	static util::Str8 type(){ return "::SubSignalType"; }
};

} // util
} // clover

#include "nodeevent.hpp"

#endif // CLOVER_NODES_SIGNALTYPETRAITS_HPP
