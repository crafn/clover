#ifndef CLOVER_GAME_INGAMELOGIC_HPP
#define CLOVER_GAME_INGAMELOGIC_HPP

#include "build.hpp"
#include "game/world_mgr.hpp"

namespace clover {
namespace game {

class InGameLogic {
public:
	InGameLogic();
	virtual ~InGameLogic();

	void update();
	void onQuit();

private:
	int32 temp;

	game::WorldMgr worldLogic;
};

} // game
} // clover

#endif // CLOVER_GAME_INGAMELOGIC_HPP
