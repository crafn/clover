#ifndef CLOVER_NODEEVENTTYPE_HPP
#define CLOVER_NODEEVENTTYPE_HPP

#include "build.hpp"
#include "resources/resource.hpp"

namespace clover {
namespace nodes {

class NodeEventType;

} // nodes
namespace resources {

template <>
struct ResourceTraits<nodes::NodeEventType> {
	RESOURCE_ID_TYPE(String);

	typedef SubCache<nodes::NodeEventType> SubCacheType;

	static util::Str8 typeName(){ return "NodeEventType"; }

	/// @todo Add SignalArgumentArray when gui is needed
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("bame"),
							AttributeDef::SignalArgumentArray("arguments"))

	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static util::Str8 identifierKey(){ return "name"; }

	static bool expired(const nodes::NodeEventType&){ return false; }
};

} // resources
namespace nodes {

class NodeEventType : public resources::Resource {
public:
	DECLARE_RESOURCE(NodeEventType)

	NodeEventType();
	virtual ~NodeEventType();

	const util::Str8& getName() const { return nameAttribute.get(); }
	const util::DynArray<SignalArgument>& getArguments() const { return argumentsAttribute.get(); }

	virtual void resourceUpdate(bool load, bool force=false);
	virtual void createErrorResource();
	
private:
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(SignalArgumentArray, argumentsAttribute)
};

} // nodes
} // clover

#endif // CLOVER_NODEEVENTTYPE_HPP