#ifndef CLOVER_GLOBAL_ENV_HPP
#define CLOVER_GLOBAL_ENV_HPP

#include "build.hpp"
#include "hardware/dll.hpp"

namespace clover {
namespace audio { class AudioMgr; }
namespace game { class WorldMgr; }
namespace physics { class PhysMgr; }
namespace resources { class Cache; }
namespace global {

/// Top level accessors for each subsystem
/// This is the global data for dll's
struct Env {
	/// @todo Add rest
	audio::AudioMgr* audioMgr;
	game::WorldMgr* worldMgr;
	physics::PhysMgr* physMgr;
	resources::Cache* resCache;
};

C_DLL_EXPORT Env g_env;

} // global
} // clover

#endif // CLOVER_GLOBAL_ENV_HPP
