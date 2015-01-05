#ifndef CLOVER_GLOBAL_ENV_HPP
#define CLOVER_GLOBAL_ENV_HPP

#include "build.hpp"
#include "hardware/dll.hpp"

namespace clover {
namespace audio { class AudioMgr; }
namespace game { class BaseGameLogic; class WorldMgr; }
namespace global { class EventMgr; }
namespace physics { class PhysMgr; }
namespace resources { class Cache; }
namespace visual { class VisualMgr; }
namespace global {

/// Top level accessors for each subsystem
struct Env {
	/// @todo Add rest
	audio::AudioMgr* audioMgr;
	game::BaseGameLogic* gameLogic;
	game::WorldMgr* worldMgr;
	global::EventMgr* eventMgr;
	physics::PhysMgr* physMgr;
	resources::Cache* resCache;
	visual::VisualMgr* visualMgr;
};

extern ENGINE_API Env* g_env;

} // global
} // clover

#endif // CLOVER_GLOBAL_ENV_HPP
