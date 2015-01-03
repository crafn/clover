#include "save_mgr.hpp"
#include "global/file.hpp"
#include "global/file_stream.hpp"
#include "util/objectnode.hpp"
#include "util/profiling.hpp"
#include "util/slicedtask.hpp"
#include "util/thread.hpp"
#include "world_mgr.hpp"
#include "worldchunk_mgr.hpp"
#include "worldentity_table.hpp"

#include <fstream>
#include <future>

/// @todo Should use FileSystem
#if PLATFORM == PLATFORM_UNIX
#include <sys/stat.h>
#include <sys/types.h>
int mkdir(const char* pathname) { return mkdir(pathname, 0755); }
#else
#include <direct.h>
#endif

namespace clover {
namespace game {

/// Abstract base for a SaveMgr task
/// @warning	Don't start asynchronous operations in the constructor because
///				tasks are queued before starting!
class SaveMgr::Task {
public:
	static constexpr real64 eternity= util::SlicedTask::eternity;

	virtual ~Task()= default;
	
	virtual void update(double time)= 0;
	
	/// Returns true if task is ready after update (no asynchronous nature)
	bool isFinished() const { return finished; }
	
protected:
	void setFinished(){ finished= true; }
	
private:
	bool finished= false;
};

/// Abstract base for tasks involving chunk clusters (save & load)
class SaveMgr::ClusterTask : public Task {
public:
	using Base= Task;

	ClusterTask(util::Atomic<WorldDataCache>& cache, ChunkSet c, ChunkSetCb on_finish)
			: cache(&cache)
			, chunks(std::move(c))
			, onFinish(std::move(on_finish)){ ensure(!chunks.empty()); }

protected:
	void setFinished(){
		Base::setFinished();
		if (onFinish)
			onFinish(chunks);
	}
	
	util::Atomic<WorldDataCache>& getCache(){ return *cache; }
	const ChunkSet& getChunks() const { return chunks; }
	
private:

	util::Atomic<WorldDataCache>* cache;
	ChunkSet chunks;
	ChunkSetCb onFinish;
};

class SaveMgr::LoadClusterTask : public ClusterTask {
	using EntityDatas= util::Map<game::WorldChunk*, util::RawPack>;
public:
	LoadClusterTask(util::Atomic<WorldDataCache>& cache, ChunkSet c, ChunkSetCb on_finish)
			: ClusterTask(cache, c, on_finish)
			, spawnTask(std::bind(&LoadClusterTask::spawnEntities, this, std::placeholders::_1)){
		PROFILE();
		for (auto& chunk : getChunks()){
			ensure(chunk);
			ensure(chunk->getState() == game::WorldChunk::State::None);
			//print(debug::Ch::Save, debug::Vb::Trivial, "LoadTask(..): (%i, %i)", chunk->getPosition().x, chunk->getPosition().y);
			chunk->setState(game::WorldChunk::State::Loading);
		}
	}

	virtual void update(double time) override {
		PROFILE();	
		switch(phase){
			case Phase::Start:
				// Start loading entity data
				try {
					dataFuture= std::async(std::launch::async, std::bind(&LoadClusterTask::loadEntityData, this));
					phase= Phase::Load;
				}
				catch (std::system_error& e){
					print(debug::Ch::Save, debug::Vb::Critical, "LoadCusterTask: std::async failed: %s", e.what());
					print(debug::Ch::Save, debug::Vb::Moderate, "Using blocking load");
					loadEntityData();
					phase= Phase::Spawn;
				}

			break;
			case Phase::Load:
				if (time == eternity || dataFuture.wait_for(util::asMicroseconds(0)) == std::future_status::ready){
					dataFuture.get();
					phase= Phase::Spawn;
				}
			break;
			
			case Phase::Spawn:
				spawnTask.runFor(time);
				if (spawnTask.isFinished())
					phase= Phase::Finish;
			break;
			
			case Phase::Finish:
				setFinished();
			break;
		}
	}

private:
	
	/// Run in thread
	void loadEntityData(){
		// Get serialized data for every chunk
		for (auto& chunk : getChunks()){
			util::DynArray<uint8> entity_data= getCache()->
				getRegion(game::GridPoint::Chunk(chunk->getPosition()).getRegionVec()).
				getEntityData(chunk->getPosition());
			util::RawPack& pack= entityDatas[chunk];
			pack.addData(entity_data.data(), entity_data.size());
			pack.seekg(0);
			pack.setReadMode();
		}
	}
	
	/// Run synchronously in thread
	void spawnEntities(const util::SlicedTask::Yield& yield){
		PROFILE();
		for (auto& chunk : getChunks()){
			util::RawPack& data_pack= entityDatas[chunk];
			
			if (data_pack.size() == 0)
				continue;
			
			while (!data_pack.eof()){
				WorldChunk::createNextEntity(data_pack, chunk);							
				yield();
			}
		}
	}
	
	enum class Phase {
		Start,
		Load,
		Spawn,
		Finish
	};
	
	Phase phase= Phase::Start;

	std::future<void> dataFuture;
	util::SlicedTask spawnTask;
	
	util::Map<game::WorldChunk*, util::RawPack> entityDatas;
};

class SaveMgr::SaveClusterTask : public ClusterTask {
public:
	SaveClusterTask(util::Atomic<WorldDataCache>& cache, ChunkSet c, ChunkSetCb on_finish)
			: ClusterTask(cache, c, on_finish)
			, serializationTask(std::bind(&SaveClusterTask::serializeChunks, this, std::placeholders::_1)){
		PROFILE();
		for (auto& chunk : getChunks()){
			ensure(chunk);
			ensure(chunk->getState() == game::WorldChunk::State::Active);
			
			chunk->setState(game::WorldChunk::State::Serializing);
		}
	}

	virtual void update(double time) override {
		PROFILE();
		serializationTask.runFor(time);

		if (serializationTask.isFinished())
			setFinished();
	}

private:

	void serializeChunks(const util::SlicedTask::Yield& yield){
		PROFILE();
		for (auto& chunk : getChunks()){
			RegionData& region_data= getCache()->getRegion(game::GridPoint::Chunk(chunk->getPosition()).getRegionVec());
			region_data.serializeChunk(*chunk);

			yield();
		}
	}
	
	util::SlicedTask serializationTask;
};

/// Transfer region from cache to hd
class SaveMgr::TransferRegionTask : public Task {
public:
	TransferRegionTask(	util::Atomic<WorldDataCache>& cache,
						RegionVec region_pos,
						const util::Str8& path)
			: task([=, &cache] (){
				transfer(cache, region_pos, path);
			}){
	}
	
	virtual void update(double time) override {
		PROFILE();
		if (!started){
			started= true;
			try {
				future= std::async(std::launch::async, task);
			}
			catch (std::system_error& e){
				print(debug::Ch::Save, debug::Vb::Critical, "TransferRegionTask: std::async failed: %s", e.what());
				print(debug::Ch::Save, debug::Vb::Moderate, "Using blocking write");
				task();
				setFinished();
			}

		}
		else if (time == eternity || future.wait_for(util::asMicroseconds(0)) == std::future_status::ready){
			future.get();
			setFinished();
		}
	}
	
private:
	static void transfer(util::Atomic<WorldDataCache>& cache, RegionVec region_pos, const util::Str8& path){
		auto& region= cache->getRegion(region_pos);
		const util::DynArray<uint8>& data= region.getData();
		
		std::ofstream file(path.cStr(), std::ios::binary);
		file.write((char*)data.data(), data.size());
		
		if (!file.good())
			print(debug::Ch::Save, debug::Vb::Critical, "Saving region file failed: %i", (int)region.getId());
		
		// Remove saved region from cache
		cache->removeRegionData(region_pos);
	}
	
	bool started= false;
	std::function<void ()> task;
	std::future<void> future;
};

void SaveMgr::setCurrentSavedGame(){
	PROFILE_("worldIo");
	mkdir("save");

	try {
		std::ifstream file((saveGamePath + "header").cStr(), std::ios::binary);
		
		util::Str8 header_data= std::string((	std::istreambuf_iterator<char>(file)),
										std::istreambuf_iterator<char>());
		
		util::ObjectNode ob;
		ob.parseText(header_data);

		auto next_id= ob.get("nextWeId").getValue<game::WorldEntityId>();
		if (next_id > gWETable.getNextUniqueId()){
			gWETable.setNextUniqueId(next_id);
		}

		dataCache->setRegionPositions(ob.get("regions").getValue<util::DynArray<RegionVec>>());
		dataCache->updateClusters(ob.get("clusters").getValue<util::Set<util::Set<ChunkVec>>>());

		loadGlobalEntities();
	}
	catch (const global::Exception& e){
		print(debug::Ch::Save, debug::Vb::Critical, "setCurrentSavedGame(..): header file couldn't be parsed");
	}

	finishedSuccesfully= false;
}

SaveMgr::SaveMgr()
		: dataCache(WorldDataCache([this](RegionData::Id id){ return loadRegion(id); })){
	saveGamePath= "save/";
	setCurrentSavedGame();
}

SaveMgr::~SaveMgr(){
	if (!finishedSuccesfully){
		print(debug::Ch::Save, debug::Vb::Critical,
				"Save was not succesfull");
	}
}

void SaveMgr::safeStartSavingChunks(ChunkSet chunks, ChunkSetCb on_finish){
	PROFILE_("worldIo");

	if (chunks.empty())
		return;

	util::Set<ChunkSet> clusters= game::gWorldMgr->getChunkMgr().getCommonClusters(chunks);

	// Make sure that clusters and regions (= header info) are up-to-date in cache
	for (const auto& cluster : clusters){
		dataCache->updateClusters({util::convertContained(cluster, [](const game::WorldChunk* ch){
			return ch->getPosition();
		})});
	
		for (const auto& chunk : cluster){
			ensure(chunk);
			util::Vec2i region_pos= game::GridPoint::Chunk(chunk->getPosition()).getRegionVec();
			dataCache->touchRegion(region_pos);
		}
	}

	//print(debug::Ch::Save, debug::Vb::Trivial, "Chunk save started");
	for (auto& cluster : game::WorldChunk::nearestSorted(priorityPosition, std::move(clusters))){
		queueSaveTask(cluster, on_finish);
	}
}

SaveMgr::ChunkSet SaveMgr::safeStartLoadingChunks(ChunkSet chunks, ChunkSetCb on_finish){
	PROFILE_("worldIo");
	util::Set<ChunkVec> positions= 
		util::convertContained(chunks, [](game::WorldChunk* ch){ return ch->getPosition(); });
	
	auto clusters_pos= dataCache->getClustersIncluding(positions);
	auto clusters=
		util::convertContained(clusters_pos, [](util::Set<ChunkVec> cluster){
			return util::convertContained(cluster, [](ChunkVec v){
				return game::gWorldMgr->getChunkMgr().getChunk(game::GridPoint::Chunk(v));
			});
		});
	
	for (auto& cluster : game::WorldChunk::nearestSorted(priorityPosition, std::move(clusters))){
		queueLoadTask(cluster, on_finish);
	}
	
	return chunks;
}

void SaveMgr::update(real32 max_time){
	PROFILE_("worldIo");
	if (taskList.empty())
		return;

	auto& task= taskList.front();
	task->update(max_time);
	
	if (task->isFinished())
		taskList.popFront();
	
	tidyCache();
}

void SaveMgr::finishSave(util::ArrayView<const WorldEntity* const> globals){
	PROFILE_("worldIo");

	saveGlobalEntities(globals);

	for (auto pos : dataCache->getCachedRegionPositions()){
		queueTransferRegionTask(pos);
	}
	
	for (auto& task : taskList){
		ensure(task);
		while (!task->isFinished())
			task->update(Task::eternity);
	}
	taskList.clear();

	writeHeaderFile();

	finishedSuccesfully= true;
}

void SaveMgr::setPriorityPosition(util::Vec2d worldpos){
	priorityPosition= game::WorldGrid::worldToChunkVec(worldpos);
}

void SaveMgr::writeHeaderFile(){
	PROFILE();
	util::ObjectNode ob;
	util::Str8 file_contents;
	{ PROFILE();	
		ob["nextWeId"].setValue(gWETable.getNextUniqueId());
		ob["regions"].setValue(dataCache->getRegionPositions());
		ob["clusters"].setValue(dataCache->getClusters());
		file_contents= ob.generateText();
	}

	std::ofstream file((saveGamePath + "header").cStr(), std::ios::binary);
	file << file_contents.cStr();

	if (!file)
		print(debug::Ch::Save, debug::Vb::Critical, "Writing header file failed");
}

void SaveMgr::loadGlobalEntities(){
	/// @todo Use global::File
	std::ifstream file(getGlobalEntitiesFilePath().cStr(), std::ios::binary);
	if (!file)
		return;
	
	file.seekg(0, std::ios::end);
	std::streamsize filesize= file.tellg();
	file.seekg(0, std::ios::beg);
	
	util::DynArray<uint8> data;
	data.resize(filesize);
	file.read((char*)data.data(), data.size());
	
	if (!file.good())
		print(debug::Ch::Save, debug::Vb::Critical, "Loading global file failed");
	
	util::RawPack data_pack;
	data_pack.addData(data.data(), data.size());
	data_pack.seekg(0);
	data_pack.setReadMode();
	
	while (!data_pack.eof()){
		WorldChunk::createNextEntity(data_pack, nullptr);
	}
}

void SaveMgr::saveGlobalEntities(util::ArrayView<const WorldEntity* const> entities){
	/// @todo Use global::File
	util::DynArray<uint8> data= WorldChunk::serializeEntities(entities);

	std::ofstream file(getGlobalEntitiesFilePath().cStr(), std::ios::binary);
	file.write((char*)data.data(), data.size());
	if (!file.good())
		print(debug::Ch::Save, debug::Vb::Critical, "Writing global file failed");
	
}

void SaveMgr::tidyCache(){
	PROFILE_("worldIo");
	const auto& cached_regions_pos= dataCache->getCachedRegionPositions();
	const auto& inhabited_regions_pos= game::gWorldMgr->getChunkMgr().getInhabitedRegionPositions();
	
	//print(debug::Ch::General, debug::Vb::Trivial, "cached %i, inhabited %i", (int)cached_regions_pos.size(), (int)inhabited_regions_pos.size());
	for (RegionVec pos : util::removed(cached_regions_pos, inhabited_regions_pos)){
		//print(debug::Ch::General, debug::Vb::Trivial, "tidying region %i %i", (int)pos.x, (int)pos.y);
		queueTransferRegionTask(pos);
	}
}

void SaveMgr::queueSaveTask(ChunkSet chunks, ChunkSetCb on_finish){
	taskList.pushBack(TaskPtr(new SaveClusterTask(dataCache, std::move(chunks), std::move(on_finish))));
}

void SaveMgr::queueLoadTask(ChunkSet chunks, ChunkSetCb on_finish){
	taskList.pushBack(TaskPtr(new LoadClusterTask(dataCache, std::move(chunks), std::move(on_finish))));
}

void SaveMgr::queueTransferRegionTask(RegionVec pos){
	taskList.pushBack(TaskPtr(new TransferRegionTask(dataCache, pos, getRegionFilePath(dataCache->getRegionId(pos)))));
}

util::DynArray<uint8> SaveMgr::loadRegion(RegionData::Id id) const {
	std::ifstream file(getRegionFilePath(id).cStr(), std::ios::binary);
	if (!file)
		return util::DynArray<uint8>();
	
	file.seekg(0, std::ios::end);
	std::streamsize filesize= file.tellg();
	file.seekg(0, std::ios::beg);
	
	util::DynArray<uint8> data;
	data.resize(filesize);
	file.read((char*)data.data(), data.size());
	
	if (!file.good())
		print(debug::Ch::Save, debug::Vb::Critical, "Loading region file failed: %i", (int)id);
	
	return data;
}

util::Str8 SaveMgr::getRegionFilePath(RegionData::Id region_id) const {
	return util::Str8::format("%s%i.region", saveGamePath.cStr(), (int)region_id);
}

util::Str8 SaveMgr::getGlobalEntitiesFilePath() const {
	return saveGamePath + "global";
}

} // game
} // clover
