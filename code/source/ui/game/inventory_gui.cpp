#include "inventory_gui.hpp"

namespace clover {
namespace ui { namespace game {

InventoryGui::InventoryGui():
	bgPanel(util::Coord::VF(0), util::Coord::VF({0.5,0.4})),
	gridLayout(util::Coord::VF(0), util::Coord::VF({0.4, 0.3})){


	addSubElement(bgPanel);
	bgPanel.addSubElement(gridLayout);

	gridLayout.setGridSize(util::Vec2i{10,7});

	setActive(false);
}

InventoryGui::~InventoryGui(){
}

void InventoryGui::add(const game::WeHandle& h){

	// Find an empty spot for entity
	util::Vec2i grid= gridLayout.getGridSize();
	for (int32 i=0; i < grid.x*grid.y; ++i){
		util::Vec2i pos= {i%grid.x, i/grid.x};
		if (gridLayout.isEmpty(pos)){
			//print(debug::Ch::Gui, debug::Vb::Trivial, "inventory pos: %i %i", pos.x ,pos.y);

			items.pushBack(gui::WeIconElement(h));
			gridLayout.addNode(items.back(), pos);
			return;
		}
	}

	ensure_msg(0, "Inventory gui is full");
}

void InventoryGui::remove(const game::WeHandle& h){

	for (auto it= items.begin(); it!= items.end(); ++it){
		if (it->getHandle() == h){
			gridLayout.removeSubElement(*it);
			items.erase(it);
			return;
		}
	}

	ensure_msg(0, "Entity not found");
}

void InventoryGui::toggle()
{

	if (isActive()){
		setActive(false);
	}
	else {
		setActive(true);
	}

}

gui::WeIconElement* InventoryGui::getTriggeredIcon(){
	for (auto &m : items){
		if (m.isSelectTriggered()){
			return &m;
		}
	}
	return 0;
}

}} // ui::game
} // clover