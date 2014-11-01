#ifndef CLOVER_GUI_INVENTORY_HPP
#define CLOVER_GUI_INVENTORY_HPP

#include "build.hpp"
#include "gui/element_panel.hpp"
#include "gui/element_layout_grid.hpp"
#include "gui/element_we_icon.hpp"
#include "game/worldentity_handle.hpp"

namespace clover {
namespace ui { namespace game {

using namespace clover::game;

class InventoryGui : public gui::Element {
public:
	InventoryGui();
	virtual ~InventoryGui();

	/// @return Returns true if adding succeeded
	void add(const game::WeHandle& h);
	void remove(const game::WeHandle& h);

	void toggle();

	gui::WeIconElement* getTriggeredIcon();

protected:
	gui::PanelElement bgPanel;
	gui::GridLayoutElement gridLayout;

	util::LinkedList<gui::WeIconElement> items;
};

}} // ui::game
} // clover

#endif // CLOVER_GUI_INVENTORY_HPP