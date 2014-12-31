#ifndef CLOVER_NODES_NODEINSTANCEGROUP_HPP
#define CLOVER_NODES_NODEINSTANCEGROUP_HPP

#include "build.hpp"
#include "nodeinstance.hpp"
#include "updateline.hpp"
#include "util/cb_listener.hpp"
#include "util/linkedlist.hpp"

#include <memory>

namespace clover {
namespace nodes {

class NodeType;
class CompositionNodeLogic;

class NodeInstanceGroup : public util::Callbacker<util::OnChangeCb> {
public:
	NodeInstanceGroup(const CompositionNodeLogicGroup& g);
	
	NodeInstanceGroup(const NodeInstanceGroup&)= delete;
	NodeInstanceGroup(NodeInstanceGroup&&)= delete;
	NodeInstanceGroup& operator=(const NodeInstanceGroup&)= delete;
	NodeInstanceGroup& operator=(NodeInstanceGroup&&)= delete;
	
	virtual ~NodeInstanceGroup();
	
	/// Updates every node once in order in which they were added
	void update();

	/// Alternative way to update
	nodes::UpdateLine getUpdateLine() const;
	
	/// True if update will do nothing
	bool isUpdateNoop() const {
		return	nodes.empty() ||
				(!groupVars.signalsSent && groupVars.updatesNeeded == 0);
	}
	
	/// Returns first found node
	NodeInstance& getNode(const util::Str8& type_name);
	
private:
	void create(const CompositionNodeLogicGroup& g);
	NodeInstance& add(const CompositionNodeLogic& comp);
	
	using NodeInstancePtr= std::unique_ptr<NodeInstance>;
	util::DynArray<NodeInstancePtr> nodes;
	util::CbListener<util::OnChangeCb> compGroupListener;
	NodeInstance::GroupVars groupVars;
	const CompositionNodeLogicGroup* compositionGroup= nullptr;
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODEINSTANCEGROUP_HPP
