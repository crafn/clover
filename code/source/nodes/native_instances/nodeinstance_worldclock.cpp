#include "game/world_mgr.hpp"
#include "nodeinstance_worldclock.hpp"

namespace clover {
namespace nodes {

void WorldClockNodeInstance::create()
{
	dayPhaseOut= addOutputSlot<SignalType::Real>("dayPhase");
	daynessOut= addOutputSlot<SignalType::Real>("dayness");
	setUpdateNeeded(true);
}

void WorldClockNodeInstance::update()
{
	real64 day_phase= game::gWorldMgr->getDayPhase();
	dayPhaseOut->send(day_phase);

	real64 dayness= day_phase*2.0;
	if (dayness > 1.0)
		dayness= 2.0 - dayness;

	daynessOut->send(dayness);
}

} // nodes
} // clover
