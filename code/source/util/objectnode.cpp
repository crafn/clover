#include "objectnode.hpp"
#include "resources/exception.hpp"

#include <json/json.h>

namespace clover {
namespace util {

ObjectNode::ObjectNode(Value t)
	: owner(nullptr)
	, ownerJsonValue(nullptr){

	if (t == Value::Array){
		jsonValue()= Json::Value(Json::arrayValue);
		ensure(isArray());
	}
	else if (t == Value::Object){
		jsonValue()= Json::Value(Json::objectValue);
		ensure(isObject());
	}

}
ObjectNode::ObjectNode(const Json::Value& v)
	: owner(nullptr)
	, ownerJsonValue(nullptr)
	, localJsonValue(v){
}

ObjectNode::ObjectNode(const ObjectNode& other)
	: owner(nullptr)
	, ownerJsonValue(nullptr){

	jsonValue()= other.jsonValue();

}

ObjectNode::ObjectNode(ObjectNode&& other)
	: owner(nullptr)
	, ownerJsonValue(nullptr){
	
	if (other.owner)
		other.owner->createReference(*this, other.jsonValue());
	else
		jsonValue()= other.jsonValue();
}

ObjectNode& ObjectNode::operator=(const ObjectNode& other){
	detachSubReferences(); // Value changes, remove references to us
	
	jsonValue()= other.jsonValue();
	return *this;
}

ObjectNode& ObjectNode::operator=(ObjectNode&& other){
	detachSubReferences(); // Value changes, remove references to us
	
	if (!owner && other.owner)
		other.owner->createReference(*this, other.jsonValue());
	else
		jsonValue()= other.jsonValue();
	return *this;
}

ObjectNode::~ObjectNode(){
	clear();
}

bool ObjectNode::isArray() const {
	return jsonValue().isArray();
}

bool ObjectNode::isObject() const {
	return (jsonValue().isObject() || jsonValue().isIntegral()) && !isArray();
}

bool ObjectNode::isNull() const {
	return jsonValue().isNull();
}

SizeType ObjectNode::size() const {
	return jsonValue().size();
}

bool ObjectNode::empty() const {
	return jsonValue().empty();
}

ObjectNode ObjectNode::get(SizeType index) const {
	const Json::Value& v= jsonValue().get(index, Json::nullValue);
	if (v.isNull())
		throw resources::ResourceException("ObjectNode::get(..): Invalid index: %i", index);
	return ObjectNode(v);
}

ObjectNode ObjectNode::operator[](SizeType index){
	ObjectNode ret;
	createReference(ret, jsonValue()[index]);
	return (ret);
}

ObjectNode ObjectNode::operator[](const util::Str8& key){
	ObjectNode ret;
	createReference(ret, jsonValue()[key.cStr()]);
	return (ret);
}

ObjectNode ObjectNode::get(const util::Str8& key) const {
	const Json::Value& v= jsonValue().get(key.cStr(), Json::nullValue);
	if (v.isNull())
		throw resources::ResourceException("ObjectNode::get(..): Key not found: %s", key.cStr());
	return ObjectNode(v);
}

ObjectNode ObjectNode::get(const util::Str8& key, Value valuetype) const {
	return ObjectNode(jsonValue().get(key.cStr(), static_cast<Json::ValueType>(valuetype)));
}

auto ObjectNode::getMemberNames() const -> MemberNames {
	auto members= jsonValue().getMemberNames();
	
	if (members.size() != size()){
		throw resources::ResourceException("ObjectNode::getMemberNames(): invalid size");
	}
	
	util::DynArray<util::Str8> ret;
	for (auto& m : members)
		ret.pushBack(m);
	return (ret);
}

void ObjectNode::parseText(const util::Str8& text){
	Json::Value root;
	Json::Reader reader;

	if(!reader.parse(text.cStr(), root)){
		throw resources::ResourceException("ObjectNode::parseText(..): Parse error: %s", reader.getFormattedErrorMessages().c_str());
	}
	
	jsonValue()= root;
}

util::Str8 ObjectNode::generateText() const {
	Json::StyledWriter writer;
	util::Str8 output_str= writer.write(jsonValue());
	return (output_str);
}

void ObjectNode::assign(Json::Value& dst, const Json::Value& src) const
{ dst= src; }

void ObjectNode::assignEmptyJsonArray(Json::Value& dst) const
{ assign(dst, Json::Value(Json::arrayValue)); }

bool ObjectNode::jsonIsNull(const Json::Value& v) const
{ return v.isNull(); }
bool ObjectNode::jsonIsArray(const Json::Value& v) const
{ return v.isArray(); }

void ObjectNode::jsonAppend(Json::Value& dst, const Json::Value& src) const
{ dst.append(src); }

bool ObjectNode::isNullJsonMember(const Json::Value& v, const char* key) const
{ return v.get(key, Json::nullValue).isNull(); }

ObjectNode ObjectNode::objNodeFromJsonMember(const Json::Value& v, const char* key) const
{ return ObjectNode(v.get(key, Json::nullValue)); }

Json::Value& ObjectNode::jsonValue()
{ return isReference() ? *ownerJsonValue : localJsonValue.get(); }

const Json::Value& ObjectNode::jsonValue() const
{ return isReference() ? *ownerJsonValue : localJsonValue.get(); }

void ObjectNode::createReference(ObjectNode& other, Json::Value& value){
	if (isReference()){
		// Create reference to owner if we have one, so that 'auto a= v[0][0]; a= something' modifies v
		owner->createReference(other, value);
	}
	else {
		references.pushBack(&other);
		other.owner= this;
		other.ownerJsonValue= &value;
	}
}

void ObjectNode::destroyReference(ObjectNode& ref){
	auto it= references.find(&ref);
	ensure(it != references.end());
	ref.owner= nullptr;
	ensure(ref.ownerJsonValue);
	ref.localJsonValue= *ref.ownerJsonValue;
	ref.ownerJsonValue= nullptr;
	references.erase(it);
}

void ObjectNode::clear(){
	if (isReference()){
		owner->destroyReference(*this);
	}
	
	detachSubReferences();
	
	jsonValue()= Json::Value(Json::Value::null);
}

void ObjectNode::detachSubReferences(){
	while (!references.empty()){
		destroyReference(*references.back());
	}
}

template <typename T>
ObjectNode ObjectNodeTraits<Quaternion<T>>::serialized(const Value& value)
{
	ObjectNode ret(ObjectNode::Value::Array);
	auto&& axis= value.axis();
	ret.append(axis.x);
	ret.append(axis.y);
	ret.append(axis.z);
	ret.append(value.rotation());
	return ret;
}

template <typename T>
auto ObjectNodeTraits<Quaternion<T>>::deserialized(const ObjectNode& node)
-> Value
{
	typename Value::Vec axis;
	for (int i= 0; i < 3; ++i)
		axis[i]= node.get(i).getValue<typename Value::Vec::Value>();
	return Value::byRotationAxis(
			axis,
			node.get(3).getValue<T>());
}

template class ObjectNodeTraits<Quaternion<real32>>;
template class ObjectNodeTraits<Quaternion<real64>>;

// InternalTraits

auto ObjectNode::InternalTraits<util::Str8>::
deserialized(const Json::Value& v) -> Value
{ return util::Str8(v.asString()); }
Json::Value ObjectNode::InternalTraits<util::Str8>::
serialized(const Value& v)
{ return Json::Value(v.cStr()); }

auto ObjectNode::InternalTraits<int64>::
deserialized(const Json::Value& v) -> Value
{ return v.asInt(); }
Json::Value ObjectNode::InternalTraits<int64>::
serialized(const Value& v)
{ return Json::Value((Value)v); }

auto ObjectNode::InternalTraits<int32>::
deserialized(const Json::Value& v) -> Value
{ return v.asInt(); }
Json::Value ObjectNode::InternalTraits<int32>::
serialized(const Value& v)
{ return Json::Value((int64)v); }

auto ObjectNode::InternalTraits<uint32>::
deserialized(const Json::Value& v) -> Value
{ return v.asInt(); }
Json::Value ObjectNode::InternalTraits<uint32>::
serialized(const Value& v)
{ return Json::Value((int64)v); }

auto ObjectNode::InternalTraits<uint64>::
deserialized(const Json::Value& v) -> Value
{ return v.asInt(); }
Json::Value ObjectNode::InternalTraits<uint64>::
serialized(const Value& v)
{ return Json::Value(v); }

auto ObjectNode::InternalTraits<real64>::
deserialized(const Json::Value& v) -> Value
{ return v.asDouble(); }
Json::Value ObjectNode::InternalTraits<real64>::
serialized(const Value& v)
{ return Json::Value(v); }

auto ObjectNode::InternalTraits<real32>::
deserialized(const Json::Value& v) -> Value
{ return v.asDouble(); }
Json::Value ObjectNode::InternalTraits<real32>::
serialized(const Value& v)
{ return Json::Value((real64)v); }

auto ObjectNode::InternalTraits<bool>::
deserialized(const Json::Value& v) -> Value
{ return v.asBool(); }
Json::Value ObjectNode::InternalTraits<bool>::
serialized(const Value& v)
{ return Json::Value(v); }

#define INTEGRAL_TRAITS(x) \
ObjectNode ObjectNodeTraits<x>::serialized(const x& value){ \
	try { \
		auto ret= ObjectNode::InternalTraits<x>::serialized(value); \
		return ret; \
	} \
	catch (std::runtime_error& e){ \
		throw resources::ResourceException("ObjectNode::serialized(..): %s: %s", #x, e.what()); \
	} \
} \
x ObjectNodeTraits<x>::deserialized(const ObjectNode& ob_node){ \
	try { \
		auto ret= ObjectNode::InternalTraits<x>::deserialized(ob_node.getJsonValue()); \
		return (ret); \
	} \
	catch (std::runtime_error& e){ \
		throw resources::ResourceException("ObjectNode::deserialized(..): %s: %s", #x, e.what()); \
	} \
}
	
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
