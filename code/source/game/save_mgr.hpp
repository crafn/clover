#ifndef CLOVER_GAME_SAVE_MGR_HPP
#define CLOVER_GAME_SAVE_MGR_HPP

#include "build.hpp"
#include "game/worldentity.hpp"
#include "game/worldgrid.hpp"
#include "util/atomic.hpp"
#include "util/math.hpp"
#include "util/pack.hpp"
#include "util/time.hpp"
#include "util/dyn_array.hpp"
#include "util/map.hpp"
#include "util/linkedlist.hpp"
#include "util/set.hpp"
#include "worlddatacache.hpp"

namespace clover {
namespace game {

class WorldChunk;

/// Handles a single savegame
class SaveMgr {
	void setCurrentSavedGame();
public:
	using ChunkSet= util::Set<game::WorldChunk*>;
	using ClusterSet= util::Set<ChunkSet>;
	using ChunkSetCb= std::function<void (const ChunkSet&)>;

	SaveMgr();
	~SaveMgr();

	/// Starts process of saving possible clusters of chunks
	void safeStartSavingChunks(ChunkSet chunks, ChunkSetCb on_finish= ChunkSetCb());
	ChunkSet safeStartLoadingChunks(ChunkSet chunks, ChunkSetCb on_finish= ChunkSetCb());

	void update(real32 max_time=0.1);
	void finishSave(util::ArrayView<const WorldEntity* const> global_entities);

	void setPriorityPosition(util::Vec2d worldpos);

protected:
	static const int32 regionWidth= game::WorldGrid::regionWidthInChunks;

	using RegionId= WorldDataCache::RegionId;

	void writeHeaderFile();
	void loadGlobalEntities();
	void saveGlobalEntities(util::ArrayView<const WorldEntity* const> entities);
	void tidyCache();

	void queueSaveTask(ChunkSet chunks, ChunkSetCb on_finish);
	void queueLoadTask(ChunkSet chunks, ChunkSetCb on_finish);
	void queueTransferRegionTask(RegionVec pos);

	/// Can be called from thread
	util::DynArray<uint8> loadRegion(RegionData::Id region_id) const;

	util::Str8 getRegionFilePath(RegionData::Id region_id) const;
	util::Str8 getGlobalEntitiesFilePath() const;

	class Task;
	class ClusterTask;
	class LoadClusterTask;
	class SaveClusterTask;
	class TransferRegionTask;

	bool finishedSuccesfully= false;
	util::Str8 saveGamePath;

	using TaskPtr= util::UniquePtr<Task>;
	util::LinkedList<TaskPtr> taskList;

	util::Atomic<WorldDataCache> dataCache;

	/// Chunks near this are prioritized
	ChunkVec priorityPosition;
};

} // game
} // clover

#endif // CLOVER_GAME_SAVE_MGR_HPP
