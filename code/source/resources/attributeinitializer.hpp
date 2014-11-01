#ifndef CLOVER_RESOURCES_ATTRIBUTE_INITIALIZER_HPP
#define CLOVER_RESOURCES_ATTRIBUTE_INITIALIZER_HPP

#include "attribute_def.hpp"
#include "build.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"

namespace clover {
namespace resources {

class BaseAttribute;
class Resource;

/// Given to Attribute as a constructor parameter
template <typename AttributeType>
class AttributeInitializer {
public:
	using Container= util::HashMap<util::Str8, BaseAttribute*>;
	using Value= typename AttributeDefImpl<AttributeType>::Value;
	
	AttributeInitializer():key(""), value(AttributeDefImpl<AttributeType>::initValue()), container(nullptr), owner(nullptr) {}
	AttributeInitializer(const util::Str8& k, const Value& v, Container& c, Resource& o)
		: key(k)
		, value(v)
		, container(&c)
		, owner(&o) {}
	
	void setKey(const util::Str8& k){ key= k; }
	const util::Str8& getKey() const { return key; }
	
	void setValue(const Value& v){ value= v; }
	const Value& getValue() const { return value; }
	
	Container* getContainer(){ return container; }
	Resource* getOwner(){ return owner; }
	
private:
	util::Str8 key;
	Value value;
	Container* container;
	Resource* owner;
};

} // resources
} // clover

#endif // CLOVER_RESOURCES_ATTRIBUTE_INITIALIZER_HPP