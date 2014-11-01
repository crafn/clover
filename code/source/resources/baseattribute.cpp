#include "baseattribute.hpp"
#include "attribute.hpp"

namespace clover {
namespace resources {

void BaseAttribute::set(const BaseAttribute& v){
	if (getType() != v.getType()){
			throw ResourceException(util::Str8::format("Attribute type mismatch for key %s: type: %s, parameter type: %s",
					getKey().cStr(),
					getType().getString().cStr(),
					v.getType().getString().cStr()).cStr());
	}
	
	switch(getType().plainAttributeType){
		
#define PLAIN_ATTRIBUTE(type) \
		case PlainAttributeType::type: \
			if (getType().isArray){ \
				typedef AttributeType<PlainAttributeType::type, true> AttribType; \
				set<AttribType>(v.get<AttribType>()); \
			} \
			else { \
				typedef AttributeType<PlainAttributeType::type, false> AttribType; \
				set<AttribType>(v.get<AttribType>()); \
			} \
			break;
#include "attributetypes.def"
#undef PLAIN_ATTRIBUTE
		default: ensure(0);
	}
}

void BaseAttribute::set(const AttributeParseInfo& parse_info){
	switch(getType().plainAttributeType){
		
		#define PLAIN_ATTRIBUTE(type) \
		case PlainAttributeType::type: \
			if (getType().isArray){ \
				typedef AttributeType<PlainAttributeType::type, true> AttribType; \
				set<AttribType>(AttributeDefImpl<AttribType>::deserialized(parse_info)); \
			} \
			else { \
				typedef AttributeType<PlainAttributeType::type, false> AttribType; \
				set<AttribType>(AttributeDefImpl<AttribType>::deserialized(parse_info)); \
			} \
			return;
		
		#include "attributetypes.def"
		#undef PLAIN_ATTRIBUTE

		default: break;

	}
	
	throw ResourceException("Invalid attribute type");
}

} // resources
} // clover