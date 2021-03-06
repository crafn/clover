#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodeinstance_we_gridgrowing.hpp"
#include "physics/grid.hpp"
#include "physics/phys_mgr.hpp"
#include "physics/world.hpp"
#include "util/time.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* WeGridGrowingNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("transform", SignalType::RtTransform2);
	n->addInputSlot("growRate", SignalType::Real, 1.0);
	n->addInputSlot("decayRate", SignalType::Real, 1.0);
	n->addInputSlot("check", SignalType::Trigger);

	n->addOutputSlot("state", SignalType::Real);
	return n;
}

template <typename T>
T smootherStep(T t)
{ return 6*t*t*t*t*t - 15*t*t*t*t + 10*t*t*t; }

void WeGridGrowingNodeInstance::create()
{
	transformIn= addInputSlot<SignalType::RtTransform2>("transform");
	growRateIn= addInputSlot<SignalType::Real>("growRate");
	decayRateIn= addInputSlot<SignalType::Real>("decayRate");
	checkIn= addInputSlot<SignalType::Trigger>("check");

	stateOut= addOutputSlot<SignalType::Real>("state");
	
	checkIn->setOnReceiveCallback(+[] (WeGridGrowingNodeInstance* self)
	{
		util::Vec2d check_pos= self->transformIn->get().translation;
		auto& grid= global::g_env.physMgr->getWorld().getGrid();
		physics::Grid::Cell& cell= grid.getCell(check_pos);
		self->grow= cell.staticPortion + cell.dynamicPortion < 0.5;

		self->setUpdateNeeded(true);
	});

	setUpdateNeeded(true);
}

void WeGridGrowingNodeInstance::update()
{
	real64 dt= global::g_env.worldMgr->getDeltaTime(); 

	real64 accel= 10.0;
	stateDeriv += grow ? accel*dt : -accel*dt;
	util::clamp(stateDeriv, -decayRateIn->get(), growRateIn->get());

	state += stateDeriv*dt;
	util::clamp(state, 0.0, 1.0);
	stateOut->send(smootherStep(state));

	if (	(!grow && state == 0.0) ||
			(grow && state == 1.0))
		setUpdateNeeded(false);
}


} // nodes
} // clover
