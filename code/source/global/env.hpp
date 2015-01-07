#ifndef CLOVER_GLOBAL_ENV_HPP
#define CLOVER_GLOBAL_ENV_HPP

#include "build.hpp"

namespace clover {
namespace audio { class AudioMgr; }
namespace debug { class Draw; class Print; }
namespace game { class BaseGameLogic; class WorldMgr; }
namespace global { class CfgMgr; class EventMgr; class FileMgr; }
namespace gui { class GuiMgr; }
namespace hardware { class Device; }
namespace physics { class PhysMgr; }
namespace resources { class Cache; }
namespace ui { namespace game { class BaseUi; }}
namespace util { class Clock; }
namespace visual { class VisualMgr; }
namespace global {

/// Top level accessors for each subsystem
struct Env {
	audio::AudioMgr* audioMgr;
	debug::Print* debugPrint;
	debug::Draw* debugDraw;
	game::BaseGameLogic* gameLogic;
	game::WorldMgr* worldMgr; // Part of gameLogic
	global::CfgMgr* cfg;
	global::EventMgr* eventMgr;
	global::FileMgr* fileMgr;
	gui::GuiMgr* guiMgr;
	hardware::Device* device;
	physics::PhysMgr* physMgr;
	resources::Cache* resCache;
	ui::game::BaseUi* ui;
	util::Clock* realClock;
	visual::VisualMgr* visualMgr;
};

extern ENGINE_API Env* g_env;

} // global
} // clover

#endif // CLOVER_GLOBAL_ENV_HPP
