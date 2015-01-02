#ifndef CLOVER_GLOBAL_ENV_HPP
#define CLOVER_GLOBAL_ENV_HPP

#include "build.hpp"

namespace clover {
namespace audio { class AudioMgr; }
namespace physics { class PhysMgr; }
namespace resources { class Cache; }
namespace global {

/// Top level accessors for each subsystem
/// This is the global data for dll's
struct Env {
	/// @todo Add rest
	audio::AudioMgr* audioMgr= nullptr;
	physics::PhysMgr* physMgr= nullptr;
	resources::Cache* resCache= nullptr;
};

extern "C" {
extern Env g_env;
}

} // global
} // clover

#endif // CLOVER_GLOBAL_ENV_HPP
