#include "basegamelogic.hpp"
#include "ingamelogic.hpp"
#include "devlogic.hpp"
#include "global/cfg_mgr.hpp"
#include "util/profiling.hpp"
#include "util/time.hpp"

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
