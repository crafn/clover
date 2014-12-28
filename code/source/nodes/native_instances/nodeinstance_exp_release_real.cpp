#include "nodeinstance_exp_release_real.hpp"

namespace clover {
namespace nodes {

void RealExpReleaseNodeInstance::create()
{
	halfValueTime= addInputSlot<SignalType::Real>("halfValueTime");
	accumIn= addInputSlot<SignalType::Real>("accumSignal");
	targetIn= addInputSlot<SignalType::Real>("target");
	realLimitIn= addInputSlot<SignalType::Real>("upperLimit");
	valueOut= addOutputSlot<SignalType::Real>("value");

	accumIn->setOnReceiveCallback([&] ()
	{
		value += accumIn->get()*util::gGameClock->getDeltaTime();
		setUpdateNeeded(true);
	});

	value= 0.0;
}

void RealExpReleaseNodeInstance::update()
{
	real64 dt= util::gGameClock->getDeltaTime();
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
