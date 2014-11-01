#ifndef CLOVER_INVENTORY_UI_HPP
#define CLOVER_INVENTORY_UI_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "inventory_gui.hpp"

namespace clover {
namespace game {

class Inventory;

} // game
namespace ui { namespace game {

/// User interface for player's inventory
/// @todo Reconsider reason for existence
class InventoryUi : public global::EventReceiver {
public:
	InventoryUi(game::Inventory& i);

	virtual void onEvent(global::Event& e);

	void update();

	void toggleGui();
	bool isGuiActive();

	game::WeHandle getTriggeredWE();

private:
	game::Inventory& inventory;
	InventoryGui inventoryGui;
};

}} // ui::game
} // clover

#endif // CLOVER_INVENTORY_UI_HPP