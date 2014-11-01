#include "gui/cursor.hpp"
#include "gui/gui_mgr.hpp"
#include "nodeinstance_guiinput.hpp"

namespace clover {
namespace nodes {

void GuiInputNodeInstance::create()
{
	channelNameIn= addInputSlot<SignalType::String>("channelName");
	cursorPos_world= addOutputSlot<SignalType::Vec2>("cursorPos_world");
	setUpdateNeeded(true);
}

void GuiInputNodeInstance::update()
{
	cursorPos_world->send(
		gui::gGuiMgr->getCursor().getPosition().
			converted(util::Coord::World).getValue());
}

} // nodes
} // clover
