#include "localplayer.hpp"
#include "audio/audiosourcehandle.hpp"
#include "audio/soundinstancehandle.hpp"
#include "ui/userinput.hpp"
#include "game/worldquery.hpp"
#include "game/world_mgr.hpp"

namespace clover {
namespace game {

LocalPlayer::LocalPlayer(){
/*	listenForEvent(global::Event::OnPlayerCreate);
	listenForEvent(global::Event::OnEntityPlacing);
	listenForEvent(global::Event::OnEntityThrow);
	listenForEvent(global::Event::OnPlayerPickupEntity);
*/
	global::Event e(global::Event::OnLocalPlayerCreate);
	e(global::Event::Object)= this;
	e.send();
}

LocalPlayer::~LocalPlayer(){
	global::Event e(global::Event::OnLocalPlayerDestroy);
	e(global::Event::Object)= this;
	e.send();
}


void LocalPlayer::update(){
	/*if (!playerWE) return;
	PlayerWE &we= *playerWE.get<PlayerWE>();



	// Pelaaja imuroi itemit testiksi

	game::WESet s= game::gWorldMgr->getQuery().getEntitiesInRadius(we.getPosition(), 1.4);
	for (auto it= s.begin(); it != s.end(); ++it){

		game::WorldEntity::Type t= (*it)->getType();
		if (t != game::WorldEntity::TouchCollectable) continue;

		// Jos keräiltävä juttu niin otetaan oikea itemi sen sisältä pois
		TouchCollectableWE* t_we= it->get<TouchCollectableWE>();
		if (!t_we) continue;
		if (!t_we->isReadyForAutoPickup()) continue;

		if (tryPickupEntity(t_we->getReceiveEntity(), false))
			t_we->popReceiveEntity();
	}

*/
}


void LocalPlayer::onEvent(global::Event& e){
/*
	switch (e.getType()){

		case global::Event::OnPlayerCreate:
			if (!playerWE){
				ensure(e(global::Event::Handle).getHandle());
				// Toistaiseksi otetaan viimeisimmäksi luotu pelaaja kontrolloitavaksi
				setPlayerWE(e(global::Event::Handle).getHandle());

				ensure(playerWE);
			}
		break;

		case global::Event::OnEntityThrow: {
			game::WeHandle h= e(global::Event::Entity).getHandle();
			ensure(h);

			if (playerWE && playerWE.get<PlayerWE>()->getUseEntity() == h)
				playerWE.get<PlayerWE>()->setUseEntity(game::WeHandle(0));

			if (!h->isPickable()){
				game::WeHandle collectable= game::gWorldMgr->getWeMgr().createEntity(game::WorldEntity::TouchCollectable, h->getPosition());
				collectable.get<TouchCollectableWE>()->setReceiveEntity(h);
				collectable.get<TouchCollectableWE>()->setAutoPickupPreventTime(1.0);
			}

			if (playerWE && playerWE.get<PlayerWE>()->getUseEntity() == h)
				playerWE.get<PlayerWE>()->setUseEntity(game::WeHandle(0));

			inventory.remove(h);

		}
		break;
		case global::Event::OnEntityPlacing: {

			game::WeHandle h= e(global::Event::Entity).getHandle();

			if (playerWE && playerWE.get<PlayerWE>()->getUseEntity() == h)
				playerWE.get<PlayerWE>()->setUseEntity(game::WeHandle(0));

			inventory.remove(h);

			if (h->isBlock()){
				// Placing-sound
				audio::AudioSourceHandle as;
				as.assignNewSource(audio::AudioSource::Type::Spatial);
				as.setPosition(h->getPosition());
				as.playSound("Place_Block0");
			}

		}
		break;

		case global::Event::OnPlayerPickupEntity: {
			game::WeHandle h= e(global::Event::Entity).getHandle();

			if (playerWE){
				// Pickup-sound
				audio::AudioSourceHandle as;
				as.assignNewSource(audio::AudioSource::Type::Spatial);
				as.setPosition(playerWE->getPosition());
				as.playSound("pickup_generic");
			}
		}
		break;

		default: break;
	}
*/
}

bool LocalPlayer::tryPickupEntity(game::WeHandle h, bool active_choice){
/*
	ensure(playerWE);

	if (!inventory.tryAdd(h)) return false;

	if (active_choice)
		playerWE.get<PlayerWE>()->setUseEntity(h);

	global::Event e(global::Event::OnPlayerPickupEntity);
	e(global::Event::Entity)= h;
	e(global::Event::Player)= playerWE;
	e(global::Event::Active)= active_choice;
	e.send();
	*/

	return true;
}


void LocalPlayer::setPlayerWE(game::WeHandle h){
/*
	if (h) ensure(h->getType() == game::WorldEntity::Player);

	if (playerWE && playerWE.get<PlayerWE>()->getInventory() == &inventory){
		playerWE.get<PlayerWE>()->setInventory(0);
	}

	playerWE= h;
	if (playerWE){
		playerWE.get<PlayerWE>()->setInventory(&inventory);
	}*/
}

} // game
} // clover