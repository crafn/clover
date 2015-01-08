#ifndef CLOVER_NODES_NODEINSTANCE_PLAYER_MGR_HPP
#define CLOVER_NODES_NODEINSTANCE_PLAYER_MGR_HPP

#include "nodes/nodeinstance.hpp"
#include "build.hpp"
#include "ui/hid/taglistentry.hpp"

namespace clover {
namespace mod {

// I'm lazy
using namespace clover::nodes;

DECLARE_NODE(PlayerMgrNode)

class PlayerMgrNode : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;

private:
	util::DynArray<ui::hid::TagListEntry> tagEntries;
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_PLAYER_MGR_HPP
