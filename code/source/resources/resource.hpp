#ifndef CLOVER_RESOURCES_RESOURCE_HPP
#define CLOVER_RESOURCES_RESOURCE_HPP

#include "build.hpp"
#include "resources/attribute.hpp"
#include "resources/attributeinitializer.hpp"
#include "resources/attribute_def.hpp"
#include "resources/baseattribute.hpp"
#include "resources/resource_id.hpp"
#include "resources/serializedresource.hpp"
#include "script/reference.hpp"
#include "util/ensure.hpp"
#include "util/hashmap.hpp"
#include "util/string.hpp"
#include "util/dyn_array.hpp"
#include "util/callbacker.hpp"

#define DECLARE_RESOURCE(type)																				   \
	virtual util::Str8 getResourceTypeName() const {																 \
		return resources::ResourceTraits<type>::typeName();													   \
	}																										   \
	virtual util::Str8 getIdentifierAsString() const {																 \
		return resources::ResourceTraits<type>::stringFromIdentifier(										   \
					getResourceAttribute(resources::ResourceTraits<type>::identifierKey())					   \
						.get<resources::ResourceTraits<type>::IdentifierAttributeType>()					   \
				);																							   \
	}																										   \
	virtual resources::BaseAttribute& getIdentifierAttribute(){												   \
		return getResourceAttribute(resources::ResourceTraits<type>::identifierKey());						   \
	}																										   \
	virtual const resources::BaseAttribute& getIdentifierAttribute() const {								   \
		return getResourceAttribute(resources::ResourceTraits<type>::identifierKey());						   \
	}																										   \
	const resources::ResourceTraits<type>::IdentifierValue& getIdentifier() const {							   \
		return getIdentifierAttribute().get<resources::ResourceTraits<type>::IdentifierAttributeType>();	   \
	}

namespace clover {
namespace resources {

/// Abstract base class for all resources in the game
/// Every resouce should be usable even if there's something like invalid file path (create a small error-resource)
///
/// Supports OnChangeCb callback listening; callback is called when
///		- attribute changes
///		- resource state changes
///		- manually calling util::OnChangeCb::trigger()
///		- when obsoletion status changes
/// If attribute change triggers resource state change, onChange is called only once, and that is when resource state changes
/// (if resource state is changed back and forth then onChange is called three times)
class Resource : public util::Callbacker<util::OnChangeCb> {
public:
	typedef uint32 Staleness;
	static constexpr Staleness Staleness_Never= -1;


	enum class State {
		Uninit,			/// Call resourceUpdate or createErrorResource, can't use resource if it's in this state
		Error,			/// Don't even try to update resource; fix attributes and hope that resource will change to some other state
		Unloaded,		/// Not loaded, resources with staleness should be usable in this state too
		Loading,
		Loaded,			/// Fully loaded
		Unloading
	};

	Resource();
	Resource(const Resource&);
	Resource(Resource&&); /// @todo throw exception if moving cached resource
	Resource& operator=(const Resource&);
	Resource& operator=(Resource&&); /// @todo throw exception if moving cached resource
	virtual ~Resource();

	///
	/// Defined by macro DECLARE_RESOURCE
	///

	virtual util::Str8 getResourceTypeName() const = 0;
	/// Gets (or generates) a string from the identifier key of the resource
	virtual util::Str8 getIdentifierAsString() const = 0;
	virtual resources::BaseAttribute& getIdentifierAttribute() = 0;
	virtual const resources::BaseAttribute& getIdentifierAttribute() const = 0;

	/// Commands resource to make itself operational or not (load or unload)
	/// Not called if getResourceState() == State::Error
	/// Not called if load == true && getResourceState() == State::Loaded
	/// Not called if load == false && getResourceState() == State::Unloaded
	/// @param load Does updating load or unload the resource
	/// @param force Load or unload immediately
	virtual void resourceUpdate(bool load, bool force=true)= 0;

	/// Sets this resource to Error-state
	/// Should never fail, so no file reading!
	virtual void createErrorResource()= 0;

	State getResourceState() const { return resourceState; }

	/// For resource cache to determine when a resource can be unloaded
	const Staleness& getStaleness() const { return staleness; }
	void updateStaleness();

	/// Called when resource is saved
	virtual void onResourceWrite(){}

	resources::BaseAttribute& getResourceAttribute(const util::Str8& key);
	const resources::BaseAttribute& getResourceAttribute(const util::Str8& key) const;

	const util::HashMap<util::Str8, resources::BaseAttribute*> getResourceAttributes() const { return attributes; }

	void setResourceFile(const ResourceFilePath& file){ resourceFile= file; }
	const ResourceFilePath& getResourceFilePath() const { return resourceFile; }

	void setResourceObsolete(bool b= true);
	bool isResourceObsolete() const { return obsolete; }

	SerializedResource getSerializedResource() const { return SerializedResource(*this); }
	void applySerializedResource(const SerializedResource& s);

	ResourceId getResourceId() const { return ResourceId(*this); }

	template <typename Archive>
	void save(Archive& ar, uint32 version) const;
	template <typename Archive>
	void load(Archive& ar, uint32 version);
	BOOST_SERIALIZATION_SPLIT_MEMBER()

protected:

	/// Remember to call when used to reset staleness
	void onResourceUse() const;

	void setResourceState(State s);

	/// Use this as constructor parameter for every attribute (by using INIT_RESOURCE_ATTRIBUTE(..) -macro)
	template <typename T>
	AttributeInitializer<T>
		attribute(const util::Str8& key, const typename AttributeDefImpl<T>::Value& default_value){
			return (AttributeInitializer<T>(key, default_value, attributes, *this));
	}

	#define RESOURCE_ATTRIBUTE(attribute_type, attribute_var) \
		resources::Attribute<resources::attribute_type ## AttributeType> attribute_var;

	#define INIT_RESOURCE_ATTRIBUTE(attribute_var, key_str, default_value) \
		attribute_var(attribute<decltype(attribute_var)::AttributeType>(key_str, default_value))

private:

	State resourceState;

	mutable Staleness staleness;

	// Is removed from resource file and shouldn't be saved any more
	bool obsolete;

	util::HashMap<util::Str8, resources::BaseAttribute*> attributes;

	ResourceFilePath resourceFile;

};

template <typename T>
class SubCache;

/// Don't use this
#define DECLARE_RESOURCE_TRAITS(type, identifier_attribute_type) \
	typedef identifier_attribute_type ## AttributeType IdentifierAttributeType; \
	typedef AttributeDefImpl<IdentifierAttributeType>::Value IdentifierValue;

/// Use this
#define RESOURCE_ID_TYPE(identifier_attribute_type) \
	typedef identifier_attribute_type ## AttributeType IdentifierAttributeType; \
	typedef AttributeDefImpl<IdentifierAttributeType>::Value IdentifierValue;

#define RESOURCE_ATTRIBUTE_DEFS(...) \
	static util::DynArray<AttributeDef> getAttributeDefs(){ return (util::DynArray<AttributeDef> { __VA_ARGS__ } ); }


template <typename T>
struct ResourceTraits;/* {
	// Implement these in specialized versions:

	DECLARE_RESOURCE_TRAITS(ClassName, String); // Last one is resource key's type ("name"'s type)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::Path("file")) // Attributes which will be in gui. First one must be identifier

	typedef SubCache<T> SubCacheType; // (Can't be named just SubCache)

	static util::Str8 typeName(){ return "Default"; } // The json section name
	static util::Str8 stringFromIdentifier(const IdentifierValue& value){ return value; } // Returns something nice which can be printed when an error occurs with resource
	static util::Str8 identifierKey(){ return "name"; } // Key of the identifier-field in json

	// Resource is unloaded if staleness gets high enough
	static bool expired(const T&){ return false; }

};*/

} // resources
namespace util {

template <>
struct TypeStringTraits<resources::Resource> {
	static util::Str8 type(){ return "::Resource"; }
};

} // util

// Implementation

template <typename Archive>
void resources::Resource::save(Archive& ar, uint32 version) const {
	util::ObjectNode ob= util::ObjectNodeTraits<SerializedResource>::serialized(getSerializedResource());
	ar & ob;
}

template <typename Archive>
void resources::Resource::load(Archive& ar, uint32 version){
	util::ObjectNode ob;
	ar & ob;
	applySerializedResource(ob.getValue<SerializedResource>());
}

} // clover

#endif // CLOVER_RESOURCES_RESOURCE_HPP