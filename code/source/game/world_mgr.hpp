#ifndef CLOVER_GAME_WORLD_MGR_HPP
#define CLOVER_GAME_WORLD_MGR_HPP

#include "build.hpp"
#include "worldchunk.hpp"
#include "worldentity_mgr.hpp"
#include "worldquery.hpp"
#include "save_mgr.hpp"
#include "worldcontactlistener.hpp"
#include "worldaudio_env.hpp"
#include "propertygrid.hpp"
#include "ui/hid/actionlistener.hpp"
#include "visual/camera.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"
#include "world_gen/world_gen.hpp"
#include "worldchunk_mgr.hpp"
#include "worldentity_handle.hpp"

namespace clover {
namespace game {

class WorldEntity;

class WorldMgr {
public:
	WorldMgr();
	virtual ~WorldMgr();

	void update();

	void setChunksLocked(bool b= true){ chunksLocked= b; }
	bool isChunksLocked() const { return chunksLocked; }

	WorldChunkMgr& getChunkMgr(){ return chunkMgr; }
	WeMgr& getWeMgr(){ return weMgr; }
	WorldQuery& getQuery(){ return worldQuery; }
	PropertyGrid& getPropertyGrid(){ return propertyGrid; }
	SaveMgr& getSaveMgr(){ return saveMgr; }
	world_gen::WorldGen& getWorldGen(){ return worldGen; }
	WorldAudioEnv& getAudioEnv(){ return audioEnv; }

	real64 getTime() const;

	/// @return Day + night duration in seconds
	real64 getDayDuration() const;

	/// @return [0, 1[, zero at midnight
	real64 getDayPhase() const;

	/// game::WorldChunk calls
	void onChunkStateChange(const game::WorldChunk& ch, WorldChunk::State prev);

private:
	void updateWorldIO();
	util::DynArray<util::Vec2i> loadedChunks;

	real64 dayTime;
	real64 lastUpdTime;
	bool chunksLocked;

	/// @todo Should be in script or somewhere else
	visual::ModelEntityDef bgDefs[3];
	visual::Entity bg[3]; // Evening, Day, Night
	visual::ModelEntityDef sunReDef;
	visual::Entity sunRE;

	visual::ModelEntityDef lightBackgroundDef;
	visual::Entity lightBackground;

	world_gen::WorldGen worldGen;

	WorldChunkMgr chunkMgr;
	WeMgr weMgr;
	WorldQuery worldQuery;
	PropertyGrid propertyGrid;
	SaveMgr	saveMgr;
	WorldAudioEnv audioEnv;

	WorldContactListener contactListener;

	ui::hid::ActionListener<> worldTimeForwardListener;
	ui::hid::ActionListener<> worldTimeRewindListener;
};

extern WorldMgr* gWorldMgr;

} // game
namespace util {

template <>
struct TypeStringTraits<game::WorldMgr> {
	static util::Str8 type(){ return "::WorldMgr"; }
};

} // util
} // clover

#endif // CLOVER_GAME_WORLD_MGR_HPP
