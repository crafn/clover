#ifndef CLOVER_NODES_NODETYPE_HPP
#define CLOVER_NODES_NODETYPE_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "script/context.hpp"
#include "script/objecttype.hpp"
#include "util/cb_listener.hpp"

namespace clover {
namespace nodes {

class NodeType;
	
} // nodes
namespace resources {

template <>
struct ResourceTraits<nodes::NodeType> {
	DECLARE_RESOURCE_TRAITS(nodes::NodeType, String)
	
	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"),
							AttributeDef::Resource("compositionLogicClass"));
	
	typedef SubCache<nodes::NodeType> SubCacheType;

	static util::Str8 typeName(){ return "NodeType"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static util::Str8 identifierKey(){ return "name"; }

	static bool expired(const nodes::NodeType&){ return false; }
};

} // resources
namespace nodes {

class NodeInstance;
class CompositionNodeLogic;
class CompositionNodeUi;

/// Defines behavior of CompositionNode and NodeInstance
class NodeType : public resources::Resource {
public:
	DECLARE_RESOURCE(NodeType)
	
	NodeType();
	virtual ~NodeType();
	
	virtual void resourceUpdate(bool load, bool force= true);
	virtual void createErrorResource();
	
	CompositionNodeLogic* createCompositionLogic(script::Context& context) const;
	CompositionNodeUi createCompositionUi() const;
	NodeInstance* createInstanceLogic(const CompositionNodeLogic& comp, script::Context& context) const;
	
	const util::Str8& getName() const { return nameAttribute.get(); }
	
private:

	script::Module* scriptModule;
	
	util::CbListener<util::OnChangeCb> moduleChangeListener;
	
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Resource, scriptModuleAttribute)
	
	
	// Composition
	script::ObjectType compositionLogicObjectType;
	RESOURCE_ATTRIBUTE(String, compositionLogicClassAttribute)
	
	// Instance
	RESOURCE_ATTRIBUTE(String, nativeInstanceClassAttribute)

	void tryStartReloading();
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODETYPE_HPP
