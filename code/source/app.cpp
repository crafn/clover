#include "app.hpp"
#include "audio/audio_mgr.hpp"
#include "debug/draw.hpp"
#include "global/env.hpp"
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

App::App(const util::Str8& executablePath)
	: env({}) // Zero-initialize
{
	global::g_env= &env;
	new debug::Print();

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

	new global::EventMgr();

	new hardware::Device();
	global::g_env->device->create(util::Str8::format("Clover Tech Preview - %s", getBuildStr()));

	util::gRealClock= new util::Clock();

	new resources::Cache();
	global::g_env->resCache->update();

	new audio::AudioMgr();
	new visual::VisualMgr();
	new debug::Draw();
	new physics::PhysMgr();
	new gui::GuiMgr();
	new ui::game::BaseUi();
	new game::BaseGameLogic();	
}

App::~App()
{
	delete global::g_env->gameLogic;
	delete global::g_env->ui;
	delete global::g_env->guiMgr;
	delete global::g_env->physMgr;
	delete global::g_env->debugDraw;
	delete global::g_env->visualMgr;
	delete global::g_env->audioMgr;
	delete global::g_env->resCache;
	delete util::gRealClock; util::gRealClock= nullptr;
	delete global::g_env->device;
	delete global::g_env->eventMgr;
	delete global::gFileMgr;

	global::destroyMemoryPools();

	delete global::gCfgMgr; global::gCfgMgr= nullptr;
}

void App::run()
{
	PROFILE();

	global::g_env->device->updateFrameTime();
	util::Clock::updateAll();

	util::Timer sleepTimer;

	while (1){
		global::SingleFrameStorage::value.clear();

		global::g_env->physMgr->preFrameUpdate();

		{ PROFILE_("sleep");
			// FPS limiter
			sleepTimer.run();

			if (util::gRealClock->getDeltaTime() < 0.014)
				global::g_env->device->sleep(0.013-util::gRealClock->getDeltaTime());
			else
				global::g_env->device->sleep(0.0001);
			
			sleepTimer.pause();
		}

		global::g_env->device->updateEvents();
		global::g_env->device->updateFrameTime();

		util::Clock::updateAll();

		global::g_env->visualMgr->getCameraMgr().update();

		// User's input update
		// Could be after game logic, but does some debug-drawing and updating debug draw in the same frame is nice
		// (if this is put after debugDraw update things are drawn twice in subsequent frames)
		// (that problem could be resolved by not drawing debug-things before they're updated once)
		if (!global::g_env->ui->update())
			break;

		global::g_env->physMgr->update();

		global::g_env->eventMgr->dispatch();
		nodes::gNodeEventMgr->dispatch();

		// Update world logic
		global::g_env->gameLogic->update();
		global::g_env->audioMgr->update();

		/// @todo Call just after gpu has finished with fluid preupdate
		global::g_env->physMgr->fluidUpdate();

		global::g_env->resCache->update();

		global::g_env->guiMgr->update();

		global::g_env->physMgr->postFrameUpdate();
		global::g_env->visualMgr->renderFrame();
	}

	global::g_env->gameLogic->onQuit();
}

} // clover
