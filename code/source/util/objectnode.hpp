#ifndef CLOVER_UTIL_OBJECTNODE_HPP
#define CLOVER_UTIL_OBJECTNODE_HPP

#include "build.hpp"
#include "util/color.hpp"
#include "util/ensure.hpp"
#include "util/math.hpp"
#include "util/objectnodetraits.hpp"
#include "util/optional.hpp"
#include "util/set.hpp"
#include "util/string.hpp"
#include "util/traits.hpp"
#include "util/vector.hpp"

#include <json/value.h>
#include <sstream>

namespace clover {
namespace util {

template <typename T>
struct IsObjectNodeSerializable;

/// Human readable graph-like serialization
/// Non-const methods that return ObjectNode use references internally, so that v["a"] = something modifies v
/// @todo const operator[] and non-const get
class ObjectNode {
public:
	typedef util::DynArray<util::Str8> MemberNames;

	enum class Value {
		Null= Json::nullValue,
		Array= Json::arrayValue,
		Object= Json::objectValue
	};

	ObjectNode(Value t= Value::Null);
	ObjectNode(const Json::Value& v);
	ObjectNode(const ObjectNode& other);
	ObjectNode(ObjectNode&& other);
	ObjectNode& operator=(const ObjectNode& other);
	ObjectNode& operator=(ObjectNode&& other);
	
	virtual ~ObjectNode();
	
	template <typename T>
	static ObjectNode create(const T& value);
	
	/// Converts raw json-value to desired type
	template <typename T>
	T getValue() const;
	
	template <typename T>
	void setValue(const T& value);
	
	/// Value as it is stored in json
	template <typename T>
	T getInternalValue() const;
	
	template <typename T>
	void setInternalValue(const T& value);

	bool isArray() const;
	bool isObject() const;
	bool isNull() const;
	
	SizeType size() const;
	bool empty() const;

	/// Array-only methods
	
	/// Adds a member to the end of array
	template <typename T>
	void append(const T& value);
	/// Throws if not found
	ObjectNode get(SizeType i) const;
	/// Returns found/created member
	ObjectNode operator[](SizeType i);
	
	/// Object-only methods
	
	/// Returns found/created member
	ObjectNode operator[](const util::Str8& key);
	/// Throws if not found
	ObjectNode get(const util::Str8& key) const;
	/// Returns value of type 'valuetype' if key not found
	ObjectNode get(const util::Str8& key, Value valuetype) const;
	/// Returns objectnode containing 'value' if key not found
	template <typename T>
	ObjectNode get(const util::Str8& key, T value) const;
	/// Returns key strings in object
	/// Indices match with get(SizeType)
	MemberNames getMemberNames() const;


	bool isReference() const { return owner != nullptr; }
	
	void parseText(const util::Str8& text);
	util::Str8 generateText() const;
	
	/// "Private"
	
	template <typename T>
	struct InternalTraits;
	
	const Json::Value& getJsonValue() const { return jsonValue(); }
	
	template <typename Archive>
	void serialize(Archive& ar, uint32 version);

private:

	// Object of which reference this is
	ObjectNode* owner;

	Json::Value* ownerJsonValue;
	Json::Value localJsonValue;
	
	Json::Value& jsonValue(){ return isReference() ? *ownerJsonValue : localJsonValue; }
	const Json::Value& jsonValue() const { return isReference() ? *ownerJsonValue : localJsonValue; }
	
	// Ugly way to achieve reference-like behaviour
	void createReference(ObjectNode& other, Json::Value& value);
	void destroyReference(ObjectNode& ref);
	void clear();
	void detachSubReferences();
	
	/// Subnodes referring to data of this object node
	util::DynArray<ObjectNode*> references;
};

template <>
struct TypeStringTraits<ObjectNode> {
	static util::Str8 type(){ return "::ObjectNode"; }
};

template <typename Archive>
void ObjectNode::serialize(Archive& ar, uint32 version) {
	if (Archive::is_saving::value) {
		util::Str8 text= generateText();
		ar & text;
	} else {
		util::Str8 text;
		ar & text;
		parseText(text);
	}
}

/// IsObjectNodeSerializable<T>::value == true if T can be stored in ObjectNode, otherwise false
template <typename T>
struct IsObjectNodeSerializable {
private:
	using Yes= int16;
	using No= int32;
	
	template <typename U> static Yes Test(decltype(&ObjectNodeTraits<U>::serialized)); /// SFINAE
	template <typename U> static No Test(...);
public:
	static const bool value= sizeof(Test<T>(0)) == sizeof(Yes);
};

template <>
struct ObjectNodeTraits<ObjectNode> {
	
	/// Serialization for generic type is made by using normal text serialization
	static ObjectNode serialized(const ObjectNode& value){
		return value;
	}
	
	static ObjectNode deserialized(const ObjectNode& ob_node){
		return ob_node;
	}
};

//
// Serialization for some common utility classes
//

template <typename T>
struct ObjectNodeTraits<
		util::DynArray<T>,
		typename std::enable_if<IsObjectNodeSerializable<T>::value>::type> {
	
	typedef util::DynArray<T> Value;
	
	static ObjectNode serialized(const Value& value){
		ObjectNode ret(ObjectNode::Value::Array); // Must initialize to array in case of values to be empty
		for (auto& m : value){
			ret.append(m);
		}
		ensure(ret.isArray());
		return ret;
	}
	
	static Value deserialized(const ObjectNode& ob_node){
		Value ret;
		for (SizeType i= 0; i<ob_node.size(); ++i){
			ret.pushBack(std::move(ob_node.get(i).getValue<T>()));
		}
		return ret;
	}
};

template <typename T>
struct ObjectNodeTraits<
		util::Set<T>,
		typename std::enable_if<IsObjectNodeSerializable<T>::value>::type> {
	typedef util::Set<T> Value;
	
	static ObjectNode serialized(const Value& value){
		ObjectNode ret(ObjectNode::Value::Array); // Must initialize to array in case of values to be empty
		for (auto& m : value){
			ret.append(m);
		}
		ensure(ret.isArray());
		return ret;
	}
	
	static Value deserialized(const ObjectNode& ob_node){
		Value ret;
		for (SizeType i= 0; i < ob_node.size(); ++i){
			ret.insert(std::move(ob_node.get(i).getValue<T>()));
		}
		return ret;
	}
};

#define OBJECT_NODE_TRAITS_VEC(x) \
template <typename T, SizeType N> \
struct ObjectNodeTraits<x<T,N>> { \
	typedef x<T,N> Value; \
	static ObjectNode serialized(const Value& value){ \
		ObjectNode ret; \
		for (SizeType i= 0; i < N; ++i) \
			ret.append<T>(value[i]); \
		return (ret); \
	} \
	static Value deserialized(const ObjectNode& node){ \
		Value ret; \
		for (SizeType i= 0; i < N && i < node.size(); ++i) \
			ret[i]= node.get(i).getValue<T>(); \
		return (ret); \
	} \
};

OBJECT_NODE_TRAITS_VEC(RealVector)
OBJECT_NODE_TRAITS_VEC(IntegerVector)

#undef OBJECT_NODE_TRAITS_VEC

template <>
struct ObjectNodeTraits<util::Color> {
	typedef util::Color Value;
	static ObjectNode serialized(const Value& value){
		ObjectNode ret;
		for (SizeType i= 0; i < Value::size(); ++i)
			ret.append<real32>(value[i]);
		return ret;
	}
	static Value deserialized(const ObjectNode& node){
		Value ret;
		for (SizeType i= 0; i < Value::size(); ++i)
			ret[i]= node.get(i).getValue<real32>();
		return ret;
	}
};

template <typename T>
struct ObjectNodeTraits<Quaternion<T>> {
	using Value= Quaternion<T>;
	static ObjectNode serialized(const Value& value){
		ObjectNode ret;
		ret["axis"].setValue(value.axis());
		ret["rotation"].setValue(value.rotation());
		return ret;
	}
	static Value deserialized(const ObjectNode& node){
		return	Value::byRotationAxis(
						node.get("axis").getValue<typename Value::Vec>(),
						node.get("rotation").getValue<T>()
				);
	}
};

template <typename S, typename R, typename T>
struct ObjectNodeTraits<util::SrtTransform<S, Quaternion<R>, T>> {
	using Value= util::SrtTransform<S, Quaternion<R>, T>;
	static ObjectNode serialized(const Value& value){
		ObjectNode ret;
		ret["scale"].setValue(value.scale);
		ret["rotation"].setValue(value.rotation);
		ret["translation"].setValue(value.translation);
		return ret;
	}
	static Value deserialized(const ObjectNode& node){
		if (!node.empty())
			return	Value(	node.get("scale").getValue<typename Value::Scale>(),
							node.get("rotation").getValue<typename Value::Rotation>(),
							node.get("translation").getValue<typename Value::Translation>()
					);
		else
			return Value(); // Return default value if null so that json can be written fast like: "offset" : null
	}
};

template <typename T>
struct ObjectNodeTraits<util::Optional<T>> {
	using Value= util::Optional<T>;
	static ObjectNode serialized(const Value& value){
		ObjectNode ret(ObjectNode::Value::Object);
		if (value)
			ret.setValue(*value);
		return ret;
	}
	static Value deserialized(const ObjectNode& ob){
		if (!ob.empty())
			return ob.getValue<T>();
		else
			return Value();
	}
};

///
/// Implementation
///

template <typename T>
ObjectNode ObjectNode::create(const T& value){
	ObjectNode ret;
	ret.setValue(value);
	return (ret);
}

template <typename T>
T ObjectNode::getValue() const {
	return (ObjectNodeTraits<T>::deserialized(*this));
}

template <typename T>
void ObjectNode::setValue(const T& value){
	jsonValue()= ObjectNodeTraits<T>::serialized(value).jsonValue();
}

/// Value as it is stored in json
template <typename T>
T ObjectNode::getInternalValue() const {
	return InternalTraits<T>::deserialized(jsonValue);
}

template <typename T>
void ObjectNode::setInternalValue(const T& value){
	jsonValue()= InternalTraits<T>::serialized(value);
}

template <typename T>
void ObjectNode::append(const T& value){
	ensure(jsonValue().isNull() || jsonValue().isArray());
	
	if (jsonValue().isNull())
		jsonValue()= Json::Value(Json::arrayValue);
		
	jsonValue().append(ObjectNodeTraits<T>::serialized(value).jsonValue());
}

template <typename T>
ObjectNode ObjectNode::get(const util::Str8& key, T value) const {
	const Json::Value& v= jsonValue().get(key.cStr(), Json::nullValue);
	if (v.isNull())
		return ObjectNode(ObjectNodeTraits<T>::serialized(value).jsonValue());
	return ObjectNode(v);
	
}

// InternalTraits

template <>
struct ObjectNode::InternalTraits<util::Str8> {
	typedef util::Str8 Value;
	static Value deserialized(const Json::Value& v){
		return util::Str8(v.asString());
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value(v.cStr());
	}
};


template <>
struct ObjectNode::InternalTraits<int64> {
	typedef int64 Value;
	static Value deserialized(const Json::Value& v){
		return v.asInt();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value((Value)v);
	}
};

template <>
struct ObjectNode::InternalTraits<int32> {
	typedef int32 Value;
	static Value deserialized(const Json::Value& v){
		return v.asInt();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value((int64)v);
	}
};

template <>
struct ObjectNode::InternalTraits<uint32> {
	typedef uint32 Value;
	static Value deserialized(const Json::Value& v){
		return v.asInt();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value((int64)v);
	}
};

template <>
struct ObjectNode::InternalTraits<uint64> {
	typedef uint64 Value;
	static Value deserialized(const Json::Value& v){
		return v.asInt();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value(v);
	}
};

template <>
struct ObjectNode::InternalTraits<real64> {
	typedef real64 Value;
	static Value deserialized(const Json::Value& v){
		return v.asDouble();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value(v);
	}
};

template <>
struct ObjectNode::InternalTraits<real32> {
	typedef real32 Value;
	static Value deserialized(const Json::Value& v){
		return v.asDouble();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value((real64)v);
	}
};

template <>
struct ObjectNode::InternalTraits<bool> {
	typedef bool Value;
	static Value deserialized(const Json::Value& v){
		return v.asBool();
	}
	
	static Json::Value serialized(const Value& v){
		return Json::Value(v);
	}
};

// Integral types

#define INTEGRAL_TRAITS(x) \
template <> \
struct ObjectNodeTraits<x> { \
	static ObjectNode serialized(const x& value); \
	static x deserialized(const ObjectNode& ob_node); \
};

INTEGRAL_TRAITS(util::Str8)
INTEGRAL_TRAITS(int64)
INTEGRAL_TRAITS(int32)
INTEGRAL_TRAITS(uint32)
INTEGRAL_TRAITS(uint64)
INTEGRAL_TRAITS(real64)
INTEGRAL_TRAITS(real32)
INTEGRAL_TRAITS(bool)

#undef INTEGRAL_TRAITS

} // util
} // clover

#endif // CLOVER_UTIL_OBJECTNODE_HPP
