#ifndef CLOVER_BASEGAMELOGIC_HPP
#define CLOVER_BASEGAMELOGIC_HPP

#include "build.hpp"

namespace clover {
namespace game {

class InGameLogic;
class DevLogic;

/// Top of the game logic
class BaseGameLogic {
public:
	BaseGameLogic();
	virtual ~BaseGameLogic();
	
	void update();
	void onQuit();

	InGameLogic*	getInGameLogic(){ return inGameLogic; }
	DevLogic*		getDevLogic(){ return devLogic; }

private:
	InGameLogic* inGameLogic;
	DevLogic* devLogic;
};

extern BaseGameLogic* gBaseGameLogic;

} // game
} // clover

#endif // CLOVER_BASEGAMELOGIC_HPP