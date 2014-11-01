#ifndef CLOVER_GLOBAL_ENV_HPP
#define CLOVER_GLOBAL_ENV_HPP

namespace clover {
namespace global {

/// Top level accessors for each subsystem
/// Semantics: all global accesses should be possible to replace
///	by giving instance of 'Env' as a parameter
/// @todo Refactor global pointers to here
struct Env {
	//audio::AudioMgr* audioMgr;
	//physics::PhysMgr* physMgr;
	//...
};

//extern Env g_env;

} // global
} // clover

#endif // CLOVER_GLOBAL_ENV_HPP