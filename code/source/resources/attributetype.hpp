#ifndef CLOVER_RESOURCES_ATTRIBUTETYPE_HPP
#define CLOVER_RESOURCES_ATTRIBUTETYPE_HPP

#include "build.hpp"
#include "util/string.hpp"

namespace clover {
namespace resources {

/// @todo Change to use same types as signals

enum class PlainAttributeType {

#define PLAIN_ATTRIBUTE(type) type,
#include "attributetypes.def"
#undef PLAIN_ATTRIBUTE

};

struct BaseAttributeType {
	virtual PlainAttributeType getPlainAttributeType() const = 0;
	virtual bool getIsArray() const = 0;
};

struct RuntimeAttributeType {
	PlainAttributeType plainAttributeType;
	bool isArray;
	
	bool operator==(const RuntimeAttributeType& other) const {
		return plainAttributeType == other.plainAttributeType && isArray == other.isArray;
	}
	
	bool operator!=(const RuntimeAttributeType& other) const { return !operator==(other); }
	
	util::Str8 getString() const{ return util::Str8::format("PlainType: %i Array: %i", (int32)plainAttributeType, isArray); }
};

template <PlainAttributeType A, bool IsArray>
struct AttributeType : public BaseAttributeType {
	static constexpr PlainAttributeType plainAttributeType= A;
	static constexpr bool isArray = IsArray;
	
	static RuntimeAttributeType runtimeType(){ return RuntimeAttributeType { plainAttributeType, isArray }; }
	static util::Str8 string(){ return runtimeType().getString(); }
	
	virtual PlainAttributeType getPlainAttributeType() const { return plainAttributeType; }
	virtual bool getIsArray() const { return isArray; }
};

#define PLAIN_ATTRIBUTE(type) \
	typedef AttributeType<PlainAttributeType::type, false> type ## AttributeType; \
	typedef AttributeType<PlainAttributeType::type, true> type ## ArrayAttributeType;
#include "attributetypes.def"
#undef PLAIN_ATTRIBUTE

} // resources
} // clover
	
#endif // CLOVER_RESOURCES_ATTRIBUTETYPE_HPP