#include "gui/cursor.hpp"
#include "gui/gui_mgr.hpp"
#include "nodeinstance_guiinput.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* GuiInputNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("channelName", SignalType::String, util::Str8("host"));
	n->addOutputSlot("cursorPos_world", SignalType::Vec2);
	return n;
}

void GuiInputNodeInstance::create()
{
	channelNameIn= addInputSlot<SignalType::String>("channelName");
	cursorPos_world= addOutputSlot<SignalType::Vec2>("cursorPos_world");
	setUpdateNeeded(true);
}

void GuiInputNodeInstance::update()
{
	cursorPos_world->send(
		global::g_env->guiMgr->getCursor().getPosition().
			converted(util::Coord::World).getValue());
}

} // nodes
} // clover
