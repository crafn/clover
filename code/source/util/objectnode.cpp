#include "objectnode.hpp"
#include "resources/exception.hpp"

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