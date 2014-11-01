#ifndef CLOVER_GAME_INVENTORY_HPP
#define CLOVER_GAME_INVENTORY_HPP

#include "build.hpp"
#include "util/dyn_array.hpp"
#include "game/worldentity_handle.hpp"

namespace clover {
namespace game {

class QuickSelect;

/// @todo Remove
class Inventory {
public:
	class Item {
	public:
		Item(game::WeHandle e);
		bool operator==(const Item& i) const {
			return entity == i.entity;
		}
		game::WeHandle getHandle(){ return entity; }

	private:
		game::WeHandle entity;
	};

	Inventory();

	/// @return True if adding succeeded
	bool tryAdd(game::WeHandle h);

	void remove(game::WeHandle h, bool drop_icon=false);

	void clear();

	void setEntitiesPosition(util::Vec2d pos);

	util::DynArray<Item>::Iter begin(){ return items.begin(); }
	util::DynArray<Item>::Iter end(){ return items.end(); }

private:
	util::DynArray<Item> items;
};

} // game
} // clover

#endif // CLOVER_GAME_INVENTORY_HPP