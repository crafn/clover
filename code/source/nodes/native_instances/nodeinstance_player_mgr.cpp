#include "nodeinstance_player_mgr.hpp"

namespace clover {
namespace nodes {

CompNode* PlayerMgrNodeInstance::compNode()
{ return new CompNode{}; }

void PlayerMgrNodeInstance::create()
{
	setUpdateNeeded(false);

	const int32 maxPlayerCount= 4;
	// Make every player character controllable
	for (int32 i= 0; i < maxPlayerCount; ++i) {
		util::Str8 player_name= util::Str8::format("player%i", i);
		tagEntries.pushBack(ui::hid::TagListEntry(player_name, "charCtrl"));
	}
}

} // nodes
} // clover
