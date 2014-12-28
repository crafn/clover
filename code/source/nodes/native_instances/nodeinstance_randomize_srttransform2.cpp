#include "nodeinstance_randomize_srttransform2.hpp"

namespace clover {
namespace nodes {

void SrtTransform2RandomizeNodeInstance::create()
{
	transformIn= addInputSlot<SignalType::SrtTransform2>("transform");
	rotBiasIn= addInputSlot<SignalType::Real>("rotBias");
	scaleBiasIn= addInputSlot<SignalType::Real>("scaleBias");
	flipXIn= addInputSlot<SignalType::Boolean>("flipX");

	transformOut= addOutputSlot<SignalType::SrtTransform2>("randomized");

	auto recv= [&] ()
	{ setUpdateNeeded(true); };
	transformIn->setOnReceiveCallback(recv);
	rotBiasIn->setOnReceiveCallback(recv);
	scaleBiasIn->setOnReceiveCallback(recv);
	flipXIn->setOnReceiveCallback(recv);

	setUpdateNeeded(false);
}

void SrtTransform2RandomizeNodeInstance::update()
{
	util::SrtTransform2d t= transformIn->get();
	
	t.rotation += util::Rand::continuous(-rotBiasIn->get(), rotBiasIn->get());
	t.scale *= 1.0 + util::Rand::continuous(-scaleBiasIn->get(), scaleBiasIn->get());

	if (flipXIn->get() && util::Rand::discrete(0, 1) == 0)
		t.scale.x *= -1.0;

	transformOut->send(t);

	setUpdateNeeded(false);
}

} // nodes
} // clover
