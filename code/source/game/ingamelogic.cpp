#include "ingamelogic.hpp"

namespace clover {
namespace game {

InGameLogic::InGameLogic(){
	game::gWorldMgr= &worldLogic;
	temp= 0;
}

InGameLogic::~InGameLogic(){
}

void InGameLogic::update(){
	temp++;
	/// @todo Fix audio bug: if audio is triggered too early it will not play :(
	if (temp == 3) worldLogic.getAudioEnv().setAmbientTrack("ambientTrack");
	
	worldLogic.update();

	localPlayer.update();

}

void InGameLogic::onQuit(){
	game::gWorldMgr->getWorldGen().generate(world_gen::WorldGen::eternity);

	// Tallennetaan kaikki ladatut chunkit kun peli sammuu
	util::Set<game::WorldChunk*> chunks= game::gWorldMgr->getChunkMgr().getChunks();

	util::Set<game::WorldChunk*> living_chunks;
	for (auto m : chunks){
		if (m->getState() == game::WorldChunk::State::Active)
			living_chunks.insert(m);
	}

	game::gWorldMgr->getSaveMgr().safeStartSavingChunks(living_chunks);

	game::gWorldMgr->getSaveMgr().finishSave(
			util::asArrayView(game::gWorldMgr->getWeMgr().getGlobalEntities()));
	game::gWorldMgr->getChunkMgr().removeAll();
	print(debug::Ch::Save, debug::Vb::Trivial, "Chunks saved on quit: %lu", (unsigned long)living_chunks.size());
}

} // game
} // clover
