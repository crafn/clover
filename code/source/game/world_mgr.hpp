#ifndef CLOVER_GAME_WORLD_MGR_HPP
#define CLOVER_GAME_WORLD_MGR_HPP

#include "build.hpp"
#include "util/dynamic.hpp"

namespace clover {
namespace nodes { class WeEdgeSpawnerNodeInstance; }
namespace game {
namespace world_gen { class WorldGen; }

class PropertyGrid;
class SaveMgr;
class WeMgr;
class WorldAudioEnv;
class WorldChunk;
class WorldChunkMgr;
class WorldEntity;
class WorldQuery;

class ENGINE_API WorldMgr {
public:
	WorldMgr();
	~WorldMgr();

	void update();

	void setChunksLocked(bool b= true);
	bool isChunksLocked() const;

	WorldChunkMgr& getChunkMgr();
	WeMgr& getWeMgr();
	WorldQuery& getQuery();
	PropertyGrid& getPropertyGrid();
	SaveMgr& getSaveMgr();
	world_gen::WorldGen& getWorldGen();
	WorldAudioEnv& getAudioEnv();

	real64 getTime() const;
	real64 getDeltaTime() const;
	real64 getTimeScale() const;

	/// @return Day + night duration in seconds
	real64 getDayDuration() const;

	/// @return [0, 1[, zero at midnight
	real64 getDayPhase() const;

	/// game::WorldChunk calls
	void onChunkStateChange(const game::WorldChunk& ch, int32 prev_state);

	/// nodes::WeEdgeSpawnerNodeInstance calls
	void onEdgeSpawnTrigger(const nodes::WeEdgeSpawnerNodeInstance& spawn);
private:
	void updateWorldIO();

	struct M;
	util::Dynamic<M> m;
};

} // game
} // clover

#endif // CLOVER_GAME_WORLD_MGR_HPP
