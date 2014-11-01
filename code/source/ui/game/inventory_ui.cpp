#include "inventory_ui.hpp"
#include "global/event.hpp"

namespace clover {
namespace ui { namespace game {

InventoryUi::InventoryUi(game::Inventory& i):
	inventory(i){
/*
	listenForEvent(global::Event::OnInventoryAdd);
	listenForEvent(global::Event::OnInventoryRemove);*/
}

void InventoryUi::onEvent(global::Event& e){
/*
	switch(e.getType()){
		case global::Event::OnInventoryAdd:
			inventoryGui.add(e(global::Event::Entity).getHandle());
		break;

		case global::Event::OnInventoryRemove:
			inventoryGui.remove(e(global::Event::Entity).getHandle());
		break;

		default: break;
	}*/
}

void InventoryUi::update(){
}

void InventoryUi::toggleGui(){
	inventoryGui.toggle();
}

bool InventoryUi::isGuiActive(){
	return inventoryGui.isActive();
}

game::WeHandle InventoryUi::getTriggeredWE(){
	if (inventoryGui.isActive()){
		gui::WeIconElement* icon= inventoryGui.getTriggeredIcon();

		if (icon){
			return icon->getHandle();
		}
	}
	return game::WeHandle();
}

}} // ui::game
} // clover