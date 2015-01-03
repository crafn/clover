#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodeinstance_worldclock.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* WorldClockNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	// Midnight: 0, morning: 0.15, midday: 0.5, evening: 0.75
	n->addOutputSlot("dayPhase", SignalType::Real);
	
	// Midnight: 0, midday: 1.0
	n->addOutputSlot("dayness", SignalType::Real);
	return n;
}

void WorldClockNodeInstance::create()
{
	dayPhaseOut= addOutputSlot<SignalType::Real>("dayPhase");
	daynessOut= addOutputSlot<SignalType::Real>("dayness");
	setUpdateNeeded(true);
}

void WorldClockNodeInstance::update()
{
	real64 day_phase= global::g_env->worldMgr->getDayPhase();
	dayPhaseOut->send(day_phase);

	real64 dayness= day_phase*2.0;
	if (dayness > 1.0)
		dayness= 2.0 - dayness;

	daynessOut->send(dayness);
}

} // nodes
} // clover
