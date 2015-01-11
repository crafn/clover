#ifndef CLOVER_NODES_NODETYPE_HPP
#define CLOVER_NODES_NODETYPE_HPP

#include "build.hpp"
#include "resources/resource.hpp"
#include "util/cb_listener.hpp"

namespace clover {
namespace nodes {

class NodeType;
	
} // nodes
namespace resources {

template <>
struct ResourceTraits<nodes::NodeType> {
	DECLARE_RESOURCE_TRAITS(nodes::NodeType, String)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"))	
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
class ENGINE_API NodeType : public resources::Resource {
public:
	DECLARE_RESOURCE(NodeType)

	NodeType();
	virtual ~NodeType();

	virtual void resourceUpdate(bool load, bool force= true);
	virtual void createErrorResource();

	CompositionNodeLogic* createCompositionLogic() const;
	CompositionNodeUi createCompositionUi() const;
	NodeInstance* createInstanceLogic(const CompositionNodeLogic& comp) const;

	/// Called from NodeInstance. Uses correct func addr even when dll is reloaded
	void updateInstance(NodeInstance& inst) const;
	void deleteInstance(NodeInstance& inst) const;

	const util::Str8& getName() const { return nameAttribute.get(); }

private:
	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Resource, moduleAttribute)
	RESOURCE_ATTRIBUTE(String, classAttribute)

	void tryStartReloading();

	using NewNodeInst= NodeInstance* ();
	using DelNodeInst= void (NodeInstance*);
	using NewNodeComp= CompositionNodeLogic* ();
	using UpdNodeInst= void (NodeInstance*);
	NewNodeInst* newNodeInst;
	DelNodeInst* delNodeInst;
	NewNodeComp* newNodeComp;
	UpdNodeInst* updNodeInst;
	util::CbListener<util::OnChangeCb> moduleChangeListener;
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODETYPE_HPP
