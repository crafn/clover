#ifndef CLOVER_GAME_LOCALPLAYER_HPP
#define CLOVER_GAME_LOCALPLAYER_HPP

#include "build.hpp"
#include "inventory.hpp"
#include "global/eventreceiver.hpp"

namespace clover {
namespace game {

/// Contains player specific info
/// @todo Remove, or replace with something more generic
class LocalPlayer : public global::EventReceiver {
public:
	LocalPlayer();
	virtual ~LocalPlayer();

	void update();
	void onEvent(global::Event& e);

	/// @return True if entity was picked up
	bool tryPickupEntity(game::WeHandle h, bool active_choice= false);
	
	void setPlayerWE(game::WeHandle h);
	game::WeHandle getPlayerWE(){ return playerWE; }

	game::Inventory &getInventory(){ return inventory; }
	
private:
	game::Inventory inventory;
	game::WeHandle playerWE;
	
};

} // game
} // clover

#endif // CLOVER_GAME_LOCALPLAYER_HPP