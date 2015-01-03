#include "ingamelogic.hpp"
#include "save_mgr.hpp"
#include "world_gen/world_gen.hpp"
#include "worldaudio_env.hpp"
#include "worldchunk.hpp"
#include "worldchunk_mgr.hpp"
#include "worldentity_mgr.hpp"

namespace clover {
namespace game {

InGameLogic::InGameLogic()
{
	game::gWorldMgr= &worldLogic;
	temp= 0;
}

InGameLogic::~InGameLogic()
{ }

void InGameLogic::update()
{
	temp++;
	/// @todo Fix audio bug: if audio is triggered too early it will not play :(
	if (temp == 3) worldLogic.getAudioEnv().setAmbientTrack("ambientTrack");
	
	worldLogic.update();
}

void InGameLogic::onQuit()
{
	// Ignore chunks not fully generated
	game::gWorldMgr->getWorldGen().stopGeneration();

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
