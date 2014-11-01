#include "objecttype.hpp"
#include "exception.hpp"
#include "util/ensure.hpp"
#include "util/string.hpp"

#include <angelscript.h>

namespace clover {
namespace script {

ObjectType::ObjectType():
	type(nullptr){
}

ObjectType::ObjectType(asIObjectType& type_):
	type(&type_){
	type->AddRef();
}

ObjectType::ObjectType(const ObjectType& other):
	type(other.type){
	
	if(type)
		type->AddRef();
}

ObjectType::ObjectType(ObjectType&& other):
	type(other.type){
	other.type= nullptr;
}

ObjectType::~ObjectType(){
	if (type)
		type->Release();
}

ObjectType& ObjectType::operator=(const ObjectType& other){
	if (&other != this){
		if (type)
			type->Release();
		type= other.type;
		if (type)
			type->AddRef();
		
	}
	return *this;
}

ObjectType& ObjectType::operator=(ObjectType&& other){
	if (&other != this){
		if(type)
			type->Release();
		type= other.type;
		other.type= nullptr;
	}
	return *this;
}

} // script
} // clover