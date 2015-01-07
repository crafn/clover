#ifndef CLOVER_RESOURCES_BASEATTRIBUTE_HPP
#define CLOVER_RESOURCES_BASEATTRIBUTE_HPP

#include "build.hpp"
#include "util/vector.hpp"
#include "resources/exception.hpp"
#include "attribute_def.hpp"
#include "attributetype.hpp"

/// @todo Replace with util::Any
#include <boost/any.hpp>
#include <functional>

namespace clover {
namespace resources {

template <typename T>
class Attribute;

/// An attribute of a Resource
class BaseAttribute {
public:
	virtual ~BaseAttribute(){}
	
	typedef std::function<void ()> OnChangeCallback;
	
	/// Returns template parameter T of Attribute<T>
	virtual const RuntimeAttributeType getType() const = 0;
	virtual const util::Str8& getKey() const = 0;
	virtual util::ObjectNode serialized() const = 0;
	
	/// Checks that type is matching and sets value
	void set(const BaseAttribute&);
	
	/// Sets from AttributeParseInfo
	void set(const AttributeParseInfo& parse_info);
	
	/// Used like: get<AttributeType::String>(), which returns util::Str8&
	template <typename T>
	const typename AttributeDefImpl<T>::Value& get() const {
		ensure(T::runtimeType() == getType());
		return static_cast<const Attribute<T>* const>(this)->get();
	}
	
	template <typename T>
	void set(const typename AttributeDefImpl<T>::Value& v){
		if (T::runtimeType() != getType()){
			throw ResourceException(util::Str8::format("Attribute type mismatch for key %s: type: %s, parameter type: %s",
					getKey().cStr(), 
					getType().getString().cStr(),
					T::string().cStr()).cStr());
		}
		
		static_cast<Attribute<T>* const>(this)->set(v);
		
	}

	
};

} // resources
} // clover

#endif // CLOVER_RESOURCES_BASEATTRIBUTE_HPP
