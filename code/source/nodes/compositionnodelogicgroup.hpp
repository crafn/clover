#ifndef CLOVER_NODES_COMPOSITIONNODELOGICGROUP_HPP
#define CLOVER_NODES_COMPOSITIONNODELOGICGROUP_HPP

#include "build.hpp"
#include "compositionnodelogic.hpp"
#include "resources/resource.hpp"
#include "util/dyn_array.hpp"
#include "util/unique_ptr.hpp"

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>

namespace clover {
namespace nodes {

class CompositionNodeLogicGroup;

} // nodes
namespace resources {

template <>
struct ResourceTraits<nodes::CompositionNodeLogicGroup> {
	DECLARE_RESOURCE_TRAITS(nodes::CompositionNodeLogicGroup, String)

	RESOURCE_ATTRIBUTE_DEFS(AttributeDef::String("name"));

	typedef SubCache<nodes::CompositionNodeLogicGroup> SubCacheType;

	static util::Str8 typeName(){ return "NodeGroup"; }
	static util::Str8 stringFromIdentifier(const IdentifierValue& key){ return key; }
	static util::Str8 identifierKey(){ return "name"; }


	static bool expired(const nodes::CompositionNodeLogicGroup&){ return false; }
};

} // resources
namespace nodes {

class NodeType;
class NodeInstanceGroup;

typedef util::UniquePtr<CompositionNodeLogic> CompositionNodeLogicPtr;

class CompositionNodeLogicGroup : public resources::Resource {
public:
	DECLARE_RESOURCE(CompositionNodeLogicGroup)

	CompositionNodeLogicGroup();

	CompositionNodeLogic& add(const NodeType& type);
	void remove(const CompositionNodeLogic& node);

	util::UniquePtr<NodeInstanceGroup> instantiate() const;

	const CompositionNodeLogic& getNode(const util::Str8& name) const;
	const util::DynArray<CompositionNodeLogicPtr>& getNodes() const { return nodes; }
	util::DynArray<CompositionNodeLogicPtr>& getNodes(){ return nodes; }

	virtual void resourceUpdate(bool load, bool force= true) override;
	virtual void createErrorResource() override;
	virtual void onResourceWrite() override;

	const util::Str8& getName() const { return nameAttribute.get(); }

private:
	/// Sort by update order so there's minimum amount of over-frame delays
	void sort() const;
	mutable bool sorted;

	bool clearingNodes;
	void clearNodes();

	/// Sets fileAttribute
	void setDefaultPath();

	// Careful with this, mutable is only for sorting
	mutable util::DynArray<CompositionNodeLogicPtr> nodes;

	util::CbMultiListener<util::OnChangeCb> nodeListener;

	RESOURCE_ATTRIBUTE(String, nameAttribute)
	RESOURCE_ATTRIBUTE(Path, fileAttribute)

	/// Used for serialization
	struct SerializationGraph;
	friend class boost::serialization::access;

	void loadFromFile();
	void saveToFile() const;


	void applySerializationGraph(const SerializationGraph& graph);
	SerializationGraph generateSerializationGraph() const;
};

} // nodes
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODELOGICGROUP_HPP
