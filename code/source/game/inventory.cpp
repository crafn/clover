#include "inventory.hpp"
#include "game/worldentity.hpp"
#include "game/worldentity_mgr.hpp"
#include "game/world_mgr.hpp"

namespace clover {
namespace game {

Inventory::Inventory(){
}

Inventory::Item::Item(game::WeHandle e):
	entity(e){
}

bool Inventory::tryAdd(game::WeHandle h){	if (!h) return false;
/*	print(debug::Ch::General, debug::Vb::Trivial, "items.size: %lu", (unsigned long)items.size());



	util::DynArray<Item>::Iter it= items.find(h);
	if (it != items.end()){
		for (auto& m : items){
			auto& e = *m.getHandle().get();
			print(debug::Ch::General, debug::Vb::Trivial, "item, id: %lu, name: %s, addr: %p, stored: %i, mounted: %i",
						(unsigned long)e.getId(), e.getDevName().cStr(), &e, e.isStored(), e.isMounted());
		}

		auto& e = *it->getHandle().get();
		print(debug::Ch::General, debug::Vb::Trivial, "it, id: %lu, name: %s, addr: %p, stored: %i, mounted: %i",
									(unsigned long)e.getId(), e.getDevName().cStr(), &e, e.isStored(), e.isMounted());
	}
	// Ei tuplia

	ensure_msg(it == items.end(), "Double add, id: %lu, name: %s, addr: %p, stored: %i, mounted: %i",
									(unsigned long)h->getId(), h->getDevName().cStr(), h.get(), h->isStored(), h->isMounted());

	if (items.size() >= 70) return false;

	print(debug::Ch::WE, debug::Vb::Trivial, "Inventory::add(..): id: %lu, name: %s, addr: %p, stored: %i, mounted: %i",
									(unsigned long)h->getId(), h->getDevName().cStr(), h.get(), h->isStored(), h->isMounted());


	WEInState s(true, h->isMounted());
	h->setState(s);
	items.pushBack(Item(h));

	global::Event e(global::Event::OnInventoryAdd);
	e(global::Event::Entity)= h;
	e.send();
*/
	return true;
}

void Inventory::remove(game::WeHandle h, bool drop_icon){/*
	if (!h) return;

	if (h->isPickable() && !drop_icon){
		WEInState s(false, false);
		h->setState(s);
	}
	if (drop_icon) {

		// Jos itemiä ei voi nostaa suoraan käteen, siitä pitää tehdä touchcollectable

		game::WeHandle tc= game::gWorldMgr->getWeMgr().createEntity(game::WorldEntity::TouchCollectable, h->getPosition());
		TouchCollectableWE* we= tc.get<TouchCollectableWE>();
		we->setReceiveEntity(h);
		we->setAutoPickupPreventTime(0.5);

	}
	
	auto it= items.find(Item(h));
	ensure(it != items.end());

	items.erase(it);

	global::Event e(global::Event::OnInventoryRemove);
	e(global::Event::Entity)= h;
	e.send();
*/
}

void Inventory::clear(){
/*
	for (auto& m : items){
		if(!m.getHandle()) continue;

		global::Event e(global::Event::OnInventoryRemove);
		e(global::Event::Entity)= m.getHandle();
		e.send();
	}
	items.clear();*/
}

void Inventory::setEntitiesPosition(util::Vec2d pos){/*
	for (auto &m : items){
		ensure(m.getHandle());
		//if (!m.getHandle()->isActive()) continue;
		ensure_msg(m.getHandle()->isStored(), 	"Id: %lld, Name: %s",
												m.getHandle()->getId(),
												m.getHandle()->getDevName().cStr());
		m.getHandle()->setPosition(pos);

	}*/
}

} // game
} // clover