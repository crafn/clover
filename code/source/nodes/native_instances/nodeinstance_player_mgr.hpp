#ifndef CLOVER_NODES_NODEINSTANCE_PLAYER_MGR_HPP
#define CLOVER_NODES_NODEINSTANCE_PLAYER_MGR_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "ui/hid/taglistentry.hpp"

namespace clover {
namespace nodes {

class PlayerMgrNodeInstance : public NodeInstance {
public:
	static CompNode* compNode();

	virtual void create() override;

private:
	util::DynArray<ui::hid::TagListEntry> tagEntries;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::PlayerMgrNodeInstance> {
	static util::Str8 type(){ return "::PlayerMgrNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_PLAYER_MGR_HPP
