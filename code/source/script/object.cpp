#include "object.hpp"
#include "util/ensure.hpp"
#include "debug/debugprint.hpp"

#include <angelscript.h>

namespace clover {
namespace script {

Object::Object():
	object(nullptr){
}

Object::Object(const ObjectType& type_, asIScriptObject& as_obj, RefType ref_type)
		: type(type_)
		, object(&as_obj)
		, refType(ref_type){
		
	tryAddRef();
}

Object::Object(Object&& other)
		: type(std::move(other.type))
		, object(other.object)
		, refType(other.refType){
	other.object= nullptr;
}

Object::~Object(){
	tryRelease();
}

Object& Object::operator=(Object&& other){
	if (&other != this){
		type= std::move(other.type);
		tryRelease();
		
		object= other.object;
		refType= other.refType;
		
		other.object= nullptr;
	}
	return *this;
}

asIScriptObject& Object::getAsObject() const {
	ensure(object);
	return *object;
}

const ObjectType& Object::getType() const {
	return type;
}

void Object::tryAddRef(){
	if (object && refType == RefType::Strong)
		object->AddRef();
}

void Object::tryRelease(){
	if (object && refType == RefType::Strong)
		object->Release();
}

} // script
} // clover