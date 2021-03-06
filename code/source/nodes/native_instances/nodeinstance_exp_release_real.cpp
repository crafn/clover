#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodeinstance_exp_release_real.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* RealExpReleaseNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("accumSignal", SignalType::Real, 0.0);
	n->addInputSlot("target", SignalType::Real, 0.0);
	n->addInputSlot("halfValueTime", SignalType::Real, 1.0);
	n->addInputSlot("upperLimit", SignalType::Real, 9999.0);
	n->addOutputSlot("value", SignalType::Real);
	return n;
}

void RealExpReleaseNodeInstance::create()
{
	halfValueTime= addInputSlot<SignalType::Real>("halfValueTime");
	accumIn= addInputSlot<SignalType::Real>("accumSignal");
	targetIn= addInputSlot<SignalType::Real>("target");
	realLimitIn= addInputSlot<SignalType::Real>("upperLimit");
	valueOut= addOutputSlot<SignalType::Real>("value");

	accumIn->setOnReceiveCallback(+[] (RealExpReleaseNodeInstance* self)
	{
		self->value += self->accumIn->get()*global::g_env.worldMgr->getDeltaTime();
		self->setUpdateNeeded(true);
	});

	value= 0.0;
}

void RealExpReleaseNodeInstance::update()
{
	real64 dt= global::g_env.worldMgr->getDeltaTime();
	value= std::min(value, realLimitIn->get());

	real64 target_value= targetIn->get();

	if (abs(value - target_value) < util::epsilon)
		setUpdateNeeded(false);

	real64 mul= std::pow(2, -dt/halfValueTime->get());
	value= (value - target_value)* mul + target_value;

	valueOut->send(value);
}

} // nodes
} // clover
