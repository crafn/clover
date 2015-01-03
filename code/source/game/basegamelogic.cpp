#include "basegamelogic.hpp"
#include "ingamelogic.hpp"
#include "devlogic.hpp"
#include "global/cfg_mgr.hpp"
#include "global/env.hpp"
#include "util/profiling.hpp"
#include "util/time.hpp"

namespace clover {
namespace game {

BaseGameLogic::BaseGameLogic(){
	if (!global::g_env->gameLogic)
		global::g_env->gameLogic= this;

	inGameLogic= new InGameLogic();
	devLogic= new DevLogic();
}

BaseGameLogic::~BaseGameLogic(){
	delete inGameLogic;
	delete devLogic;

	if (global::g_env->gameLogic == this)
		global::g_env->gameLogic= nullptr;
}

void BaseGameLogic::update(){
	PROFILE();

	devLogic->update();

	if (inGameLogic)
		inGameLogic->update();
}

void BaseGameLogic::onQuit(){
	if (inGameLogic)
		inGameLogic->onQuit();
}

} // game
} // clover
