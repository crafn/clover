#include "app.hpp"
#include "audio/audio_mgr.hpp"
#include "debug/debugdraw.hpp"
#include "global/cfg_mgr.hpp"
#include "global/event_mgr.hpp"
#include "global/memory.hpp"
#include "game/worldquery.hpp"
#include "game/worldchunk.hpp"
#include "game/worldentity_mgr.hpp"
#include "game/worldentity_handleconnection.hpp"
#include "game/save_mgr.hpp"
#include "game/world_mgr.hpp"
#include "game/basegamelogic.hpp"
#include "game/ingamelogic.hpp"
#include "gui/gui_mgr.hpp"
#include "hardware/device.hpp"
#include "hardware/memory.hpp"
#include "nodes/nodeevent_mgr.hpp"
#include "physics/phys_mgr.hpp"
#include "resources/cache.hpp"
#include "ui/game/base_ui.hpp"
#include "ui/game/editor/editor_ui.hpp"
#include "util/pack.hpp"
#include "util/string.hpp"
#include "global/file_mgr.hpp"
#include "global/file.hpp"
#include "util/profiling.hpp"
#include "visual/entity_mgr.hpp"
#include "visual/visual_mgr.hpp"
#include "visual/camera_mgr.hpp"

#include <thread>
#include <list>

namespace clover {

App::App(const util::Str8& executablePath){
	print(debug::Ch::General, debug::Vb::Trivial, getBuildStr());

	// Initial cfg loading should be as early as possible, 
	// as its needed for heap creation
	global::gCfgMgr= new global::CfgMgr();

	// Creates custom heap if custom new is enabled in hardware/memory.cpp
	hardware::createHeap(
			global::gCfgMgr->get<SizeType>("hardware::heapSize"),
			global::gCfgMgr->get<SizeType>("hardware::heapBlocks"));

	global::createMemoryPools(
			global::gCfgMgr->get<SizeType>("global::singleFrameMemory"));

	auto filemgr = new global::FileMgr();
	global::File bin(executablePath);
	filemgr->add(bin.getAbsoluteDirname() + "/../../resources/gamedata", true, 10);
	filemgr->add(bin.getAbsoluteDirname() + "/../resources/gamedata", true, 20);
	filemgr->add(bin.getAbsoluteDirname() + "/data", false, 30);

	/// @todo Add this when multiple cfg files are supported
	//global::gCfgMgr->loadAdditionalCfgs();

	global::gEventMgr= new global::EventMgr();

	hardware::gDevice= new hardware::Device();
	hardware::gDevice->create(util::Str8::format("Clover Tech Preview - %s", getBuildStr()));

	util::gRealClock= new util::Clock();
	util::gGameClock= new util::Clock();

	new resources::Cache();
	resources::gCache->update();

	new audio::AudioMgr();
	new visual::VisualMgr();
	debug::gDebugDraw= new debug::DebugDraw();
	physics::gPhysMgr= new physics::PhysMgr();
	gui::gGuiMgr= new gui::GuiMgr();
	ui::game::gBaseUi= new ui::game::BaseUi();
	game::gBaseGameLogic= new game::BaseGameLogic();	
}

App::~App(){
	delete game::gBaseGameLogic; game::gBaseGameLogic= nullptr;
	delete ui::game::gBaseUi; ui::game::gBaseUi= nullptr;
	delete gui::gGuiMgr; gui::gGuiMgr= nullptr;
	delete physics::gPhysMgr; physics::gPhysMgr= nullptr;
	delete debug::gDebugDraw; debug::gDebugDraw= nullptr;
	delete visual::gVisualMgr;
	delete audio::gAudioMgr;
	delete resources::gCache; resources::gCache= nullptr;
	delete util::gGameClock; util::gGameClock= nullptr;
	delete util::gRealClock; util::gRealClock= nullptr;
	delete hardware::gDevice; hardware::gDevice= nullptr;
	delete global::gEventMgr; global::gEventMgr= nullptr;
	delete global::gFileMgr;

	global::destroyMemoryPools();

	delete global::gCfgMgr; global::gCfgMgr= nullptr;
}

void App::run(){
	PROFILE();

	hardware::gDevice->updateFrameTime();
	util::Clock::updateAll();
	
	util::Timer sleepTimer;

	while (1){
		global::SingleFrameStorage::value.clear();

		physics::gPhysMgr->preFrameUpdate();

		{ PROFILE_("sleep");
			// FPS limiter
			sleepTimer.run();

			if (util::gRealClock->getDeltaTime() < 0.014)
				hardware::gDevice->sleep(0.013-util::gRealClock->getDeltaTime());
			else
				hardware::gDevice->sleep(0.0001);
			
			sleepTimer.pause();
		}

		hardware::gDevice->updateEvents();
		hardware::gDevice->updateFrameTime();

		util::Clock::updateAll();

		visual::gVisualMgr->getCameraMgr().update();

		// User's input update
		// Could be after game logic, but does some debug-drawing and updating debug draw in the same frame is nice
		// (if this is put after debugDraw update things are drawn twice in subsequent frames)
		// (that problem could be resolved by not drawing debug-things before they're updated once)
		if (!ui::game::gBaseUi->update())
			break;

		if (!util::gGameClock->isPaused()){
			physics::gPhysMgr->update();

			global::gEventMgr->dispatch();
			nodes::gNodeEventMgr->dispatch();

			// Update world logic
			game::gBaseGameLogic->update();
			audio::gAudioMgr->update();

			/// @todo Call just after gpu has finished with fluid preupdate
			physics::gPhysMgr->fluidUpdate();
		}

		resources::gCache->update();

		gui::gGuiMgr->update();

		physics::gPhysMgr->postFrameUpdate();
		visual::gVisualMgr->renderFrame();
	}

	game::gBaseGameLogic->onQuit();
}

} // clover
