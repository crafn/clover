#ifndef CLOVER_GLOBAL_ENV_HPP
#define CLOVER_GLOBAL_ENV_HPP

#include "build.hpp"

namespace clover {
namespace audio { class AudioMgr; }
namespace debug { class Draw; class Print; }
namespace game { class BaseGameLogic; class WorldMgr; }
namespace global { class EventMgr; }
namespace hardware { class Device; }
namespace physics { class PhysMgr; }
namespace resources { class Cache; }
namespace visual { class VisualMgr; }
namespace global {

/// Top level accessors for each subsystem
struct Env {
	/// @todo Add rest
	audio::AudioMgr* audioMgr;
	debug::Print* debugPrint;
	debug::Draw* debugDraw;
	game::BaseGameLogic* gameLogic;
	game::WorldMgr* worldMgr;
	global::EventMgr* eventMgr;
	hardware::Device* device;
	physics::PhysMgr* physMgr;
	resources::Cache* resCache;
	visual::VisualMgr* visualMgr;
};

extern ENGINE_API Env* g_env;

} // global
} // clover

#endif // CLOVER_GLOBAL_ENV_HPP
