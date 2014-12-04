#include "world_mgr.hpp"
#include "audio/audio_mgr.hpp"
#include "global/cfg_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "hardware/device.hpp"
#include "physics.hpp"
#include "physics/phys_mgr.hpp"
#include "physics/world.hpp"
#include "save_mgr.hpp"
#include "util/containeralgorithms.hpp"
#include "visual/visual_mgr.hpp"
#include "visual/entity_mgr.hpp"
#include "visual/camera_mgr.hpp"
#include "world_gen/workertype.hpp"
#include "worldchunk.hpp"
#include "worldentity.hpp"
#include "worldentity_mgr.hpp"
#include "worldentity_set.hpp"
#include "worldquery.hpp"

// For debug
#include "hardware/keyboard.hpp"

// For grid test
#include "physics/chunk_util.hpp"

namespace clover {
namespace game {

WorldMgr* gWorldMgr;

WorldMgr::WorldMgr()
	: lastUpdTime(-std::numeric_limits<real64>::infinity())
	, chunksLocked(false)
	, propertyGrid(physics::gPhysMgr->getWorld().getGrid())
	, worldGen(*this, resources::gCache->getSubCache<world_gen::WorkerType>().getResources())
	, worldTimeForwardListener("host", "dev", "worldTimeForward", [this] (){
		dayTime += util::gGameClock->getDeltaTime()*300;
	})
	, worldTimeRewindListener("host", "dev", "worldTimeRewind", [this] (){
		dayTime -= util::gGameClock->getDeltaTime()*300;
	})
{

	dayTime= getDayDuration()*0.45; // Start from day

	bgDefs[0].setModel("background_sky_evening");
	bgDefs[1].setModel("background_sky_day");
	bgDefs[2].setModel("background_sky_night");

	for (uint32 i= 0; i < 3; ++i){
		bgDefs[i].setEnvLight(1.0);
		bgDefs[i].useOnlyEnvLight();
		
		bg[i].setDef(bgDefs[i]);
		bg[i].setScale(util::Vec3d(1000000));
		bg[i].setPosition(util::Vec3d({0, 0, -1000000}));
		bg[i].setDrawPriority(i);
	}

	lightBackgroundDef.setModel("lightBackground");
	lightBackgroundDef.setEnvLight(1.0);
	lightBackgroundDef.setLightAlphaAdd(0.3);
	lightBackgroundDef.setColorMul(util::Color{1.0f,0.7f,0.3f,0});
	lightBackground.setDef(lightBackgroundDef);
	lightBackground.setScale(util::Vec3d(1));
	lightBackground.setDrawPriority(-100);
	lightBackground.setCoordinateSpace(util::Coord::View_Stretch);

	sunReDef.setModel("background_sun");

	sunReDef.useOnlyEnvLight();
	sunReDef.setEnvLight(1.0);
	sunRE.setDef(sunReDef);

	visual::TriMesh gtData;
	gtData.addRectByCenter(util::Vec2f(0),util::Vec2f(2));
}

WorldMgr::~WorldMgr()
{ }

void WorldMgr::update()
{
	{ // Inform grid changes to WorldEntities
		weMgr.removeFlagged(); // Removes phys objects flagged by ui

		auto& grid= physics::gPhysMgr->getWorld().getGrid();
		auto&& chunk_positions= grid.getChunkPositions();
		for (auto& ch_pos : chunk_positions) {
			util::ArrayView<const physics::Grid::Cell> cells=
				grid.getChunkCells(ch_pos);
			uint32 width= grid.getChunkWidth();
			uint32 width_c= grid.getChunkWidth()*grid.getCellsInUnit();
			real32 half_cell= 0.5f/grid.getCellsInUnit();
			for (SizeType i= 0; i < cells.size(); ++i) {
				if (cells[i].worldEdge)
					continue;

				if (	lastUpdTime > cells[i].lastStaticEdit &&
						lastUpdTime > cells[i].lastDynamicEdit)
					continue;
			
				for (physics::Object* obj : cells[i].objects) {
					if (!obj)
						continue;
					WorldEntity* we= game::getOwnerWe(*obj);
					if (!we || !we->hasAttribute("gridChange"))
						continue;

					we->setAttribute("gridChange", nodes::TriggerValue{});
				}
			}
		}
	}
	updateWorldIO();
	weMgr.spawnNewEntities(); // Created entities need to be updated this frame

	weMgr.update();
	weMgr.spawnNewEntities();
	weMgr.removeFlagged();

	// Modify grid for loaded chunks so that no changes are seen
	// This prevents e.g. creating double edges for ground on world load
	/// @todo Fix - this doesn't prevent seeing changes in weMgr.update.
	///       Possible solution could be that spawnNewEntities runs node init,
	///       and this extra chunk update would be done before weMgr.update
	for (auto p : loadedChunks)
		physics::gPhysMgr->getWorld().getGrid().touchCells(p);
	loadedChunks.clear();


	visual::Camera& camera= visual::gVisualMgr->getCameraMgr().getSelectedCamera();
	
	{
		lightBackground.setPosition(util::Vec3d(0));

		real32 phase= getDayPhase() + 0.75;
		if (phase >= 1.0)
			phase -= 1.0;
		
		// [0, 1]
		real32 illumination= pow(sin(phase*util::tau), 2);

		if (phase > 0.5)
			illumination= 0;

		// Day light
		util::Color c_cur = util::Color{illumination*1.3f, illumination*1.3f, illumination*1.1f};

		// Bluish night
		real32 bluemul=	 pow(util::abs(sin(phase*util::tau)), 0.5);

		if (phase < 0.5) bluemul= 0;

		c_cur += util::Color{0.1f, 0.1f, 0.6f} * bluemul;

		real32 redmul= pow((- pow(illumination-0.3f, 2) + 0.08f), 3)*2000.f;

		// Red in morning/evening
		if (redmul > 0 )
			c_cur += util::Color{0.8f, 0.3f, 0.0f} * redmul;

		bgDefs[1].setColorMul(util::Color{1,1,1,illumination});
		bgDefs[2].setColorMul(util::Color{1,1,1,bluemul});

		// Sun
		util::Vec2d sun_pos= {cos(phase*util::tau), (sin(phase*util::tau)-0.5)};
		const real64 depth= 10000.0;
		sunRE.setPosition(util::Vec3d{sun_pos.x*depth, sun_pos.y*depth, -depth});
		sunRE.setScale(util::Vec2d(0.2*depth + (cos(phase*util::tau*2) + 1)*0.1*depth).xyz()*0.7);

		real32 mul= (real32)pow((0.5*(sin(phase*util::tau*2 - util::pi*0.5) + 1)), 0.5);
		sunReDef.setColorMul(util::Color{1.f, 1.0f*mul, 1.0f*mul, 1.0f} * (1+ (1-mul)*0.2));

		visual::gVisualMgr->getEntityMgr().setEnvLight(c_cur, -util::Vec2f{(real32)cos(phase*util::tau), (real32)sin(phase*util::tau)});
	}
	
	dayTime += util::gGameClock->getDeltaTime();
	lastUpdTime= util::gGameClock->getTime();
}

real64 WorldMgr::getTime() const
{ return util::gGameClock->getTime(); }

real64 WorldMgr::getDayDuration() const {
	return 60*20;
}

real64 WorldMgr::getDayPhase() const {
	real64 intpart;
	return modf(dayTime/getDayDuration(), &intpart);
}

void WorldMgr::onChunkStateChange(const game::WorldChunk& ch, WorldChunk::State prev){
	if (prev == WorldChunk::State::Loading) {
		loadedChunks.pushBack(ch.getPosition());
	}

	if (ch.getState() == game::WorldChunk::State::Active){
		physics::gWorld->addChunk(ch.getPosition(), WorldGrid::chunkWidthInBlocks);
	}
	else if (ch.getState() == game::WorldChunk::State::Destroying){
		physics::gWorld->removeChunk(ch.getPosition(), WorldGrid::chunkWidthInBlocks);
	}
}

void WorldMgr::updateWorldIO(){
	if (chunksLocked) {
		// Generate only stuff for already playable chunks
		worldGen.generate(0.0);
		return;
	}

	visual::Camera& camera= visual::gVisualMgr->getCameraMgr().getSelectedCamera();
	
	WorldVec priority_pos= camera.getPosition();
	saveMgr.setPriorityPosition(priority_pos);
	
	util::Set<game::WorldChunk*> new_chunks= chunkMgr.inhabitChunksInRadius(camera.getPosition(), global::gCfgMgr->get<int32>("game::worldSimulationRadius", 20));

	for (auto m : new_chunks){
		//print(debug::Ch::WorldChunk, debug::Vb::Trivial, "New chunk (%i, %i)", m->getPosition().x, m->getPosition().y);
		ensure(m->getState() != game::WorldChunk::State::Destroying);
	}

	if (new_chunks.size()){
		//print(debug::Ch::WorldChunk, debug::Vb::Trivial, "%lu chunks created", (unsigned long)new_chunks.size());

		saveMgr.safeStartLoadingChunks(new_chunks, [this] (const SaveMgr::ChunkSet& cluster){
			// If a chunk is activated and destroyed in the same frame, handle isn't always created
			// and dependencies wouldn't be saved without this
			game::WeHandle::fixLostHandles();
			
			// Activate chunks
			for (auto& chunk : cluster){
				chunk->setState(WorldChunk::State::Active);
			}
		});
		
		util::extractIf(new_chunks, [](game::WorldChunk* ch){
			return ch->getState() == WorldChunk::State::Loading;
		});
		
		if (!new_chunks.empty())
			worldGen.addToGeneration(new_chunks, priority_pos);
	}	
	
	// Streaming

	worldGen.generate(global::gCfgMgr->get<double>("game::maxWorldGeneratingTime"));
	saveMgr.update(global::gCfgMgr->get<double>("game::maxWorldIoTime"));

	// Saving

	util::Set<game::WorldChunk*> out_chunks= chunkMgr.getChunksOutRadius(camera.getPosition(), 100.0);
	
	util::extractIf(out_chunks, [] (game::WorldChunk* c){
		return c->getState() != game::WorldChunk::State::Active;
	});

	if (!out_chunks.empty()){
		chunkMgr.updateChunkDependencies();

		saveMgr.safeStartSavingChunks(out_chunks, [this] (const SaveMgr::ChunkSet& cluster){
			//print(debug::Ch::Save, debug::Vb::Trivial, "Chunk cluster saved");
			chunkMgr.startDestroyingChunks(cluster); // Destroy chunks when saved
		});
	}
	
	/// @todo Account for streaming time
	chunkMgr.updateDestroying();
}

} // game
} // clover
