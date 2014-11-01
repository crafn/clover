#include "resource.hpp"

namespace clover {
namespace resources {

Resource::Resource():
	resourceState(State::Uninit),
	staleness(0),
	obsolete(false),
	resourceFile("", ""){

}

Resource::Resource(const Resource& other):
	Callbacker(other),
	resourceState(other.resourceState),
	staleness(other.staleness),
	obsolete(other.obsolete),
	attributes(other.attributes),
	resourceFile(other.resourceFile){
	
}

Resource::Resource(Resource&& other):
	Callbacker(std::move(other)),
	resourceState(other.resourceState),
	staleness(other.staleness),
	obsolete(other.obsolete),
	attributes(other.attributes), // Don't move, it gets Attributes messed up
	resourceFile(other.resourceFile){
	
}

Resource::~Resource(){
}

Resource& Resource::operator=(const Resource& other){
	Callbacker::operator=(other);
	resourceState= other.resourceState;
	staleness= other.staleness;
	attributes= other.attributes;
	resourceFile= other.resourceFile;
	
	return *this;
}

Resource& Resource::operator=(Resource&& other){
	Callbacker::operator=(std::move(other));
	resourceState= other.resourceState;
	staleness= other.staleness;
	attributes= other.attributes; // Don't move, it gets Attributes messed up
	resourceFile= other.resourceFile;
	
	return *this;
}

void Resource::updateStaleness(){
	++staleness;
}

void Resource::onResourceUse() const {
	staleness= 0;
}

void Resource::setResourceState(State s){
	State old= resourceState;
	resourceState= s;
	if (resourceState != old)
		util::OnChangeCb::trigger();
}

BaseAttribute& Resource::getResourceAttribute(const util::Str8& key){
	auto it= attributes.find(key);
	
	if (it == attributes.end())
		throw ResourceException(util::Str8::format("ResourceAttribute with key %s not found", key.cStr()).cStr());
		
	return *it->second;
}

const BaseAttribute& Resource::getResourceAttribute(const util::Str8& key) const {
	auto it= attributes.find(key);
	
	if (it == attributes.end())
		throw ResourceException(util::Str8::format("ResourceAttribute with key %s not found", key.cStr()).cStr());
		
	return *it->second;
}

void Resource::setResourceObsolete(bool b){
	bool prev= obsolete;
	obsolete= b;
	if (prev != obsolete)
		util::OnChangeCb::trigger();
}

void Resource::applySerializedResource(const SerializedResource& s){
	if (s.getTypeName() != getResourceTypeName())
		throw ResourceException("applySerializedResource: type names doesn't match");
	
	setResourceFile(s.getPath());
	
	util::ObjectNode attribs= s.getResourceAttributes();
	for (SizeType i= 0; i < attribs.size(); ++i){
		util::Str8 name= attribs.getMemberNames()[i];
		const util::ObjectNode& ob= attribs[name];
		AttributeParseInfo info(ob, s.getPath().directoryFromRoot());
		
		getResourceAttribute(name).set(info);
	}
}

} // resources
} // clover