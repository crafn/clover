#include "basegamelogic.hpp"
#include "ingamelogic.hpp"
#include "devlogic.hpp"
#include "global/cfg_mgr.hpp"
#include "util/profiling.hpp"

namespace clover {
namespace game {

BaseGameLogic* gBaseGameLogic= 0;

BaseGameLogic::BaseGameLogic(){
	inGameLogic= new InGameLogic();
	devLogic= new DevLogic();
}

BaseGameLogic::~BaseGameLogic(){
	delete inGameLogic;
	delete devLogic;
}

void BaseGameLogic::update(){
	PROFILE();
	if (global::gCfgMgr->get<bool>("game::useFixedTimeStep", false)){
		util::gGameClock->setFixedDeltaTime(global::gCfgMgr->get<real64>("game::fixedTimeStep", 1.0/60.0));
	}
	else {
		util::gGameClock->unsetFixedDeltaTime();
	}
	
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
