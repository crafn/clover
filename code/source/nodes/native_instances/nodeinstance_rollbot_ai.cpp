#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodeinstance_rollbot_ai.hpp"
#include "util/time.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* RollBotAiNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("acceleration", SignalType::RtTransform2);
	n->addInputSlot("angularVelocity", SignalType::Real, 0.0);
	n->addInputSlot("torqueMul", SignalType::Real, 1.0);
	n->addInputSlot("wakeUp", SignalType::Trigger);
	n->addInputSlot("timeAwake", SignalType::Real, 1.0);
	n->addInputSlot("escape", SignalType::Vec2);
	n->addInputSlot("position", SignalType::Vec2);

	n->addOutputSlot("torque", SignalType::Real);
	n->addOutputSlot("awake", SignalType::Boolean);
	return n;
}

void RollBotAiNodeInstance::create()
{
	aIn= addInputSlot<SignalType::RtTransform2>("acceleration");
	omegaIn= addInputSlot<SignalType::Real>("angularVelocity");
	torqueMulIn= addInputSlot<SignalType::Real>("torqueMul");
	wakeUpIn= addInputSlot<SignalType::Trigger>("wakeUp");
	timeAwakeIn= addInputSlot<SignalType::Real>("timeAwake");
	escapeIn= addInputSlot<SignalType::Vec2>("escape");
	positionIn= addInputSlot<SignalType::Vec2>("position");

	torqueOut= addOutputSlot<SignalType::Real>("torque");
	awakeOut= addOutputSlot<SignalType::Boolean>("awake");

	wantedDirection= 0;
	frustration= 0.0;
	lastOmega= 0.0;
	hardPushTimeout= 0.0;
	thinkingTimeout= 0.0;
	thinkingProbability= 0.0;
	startSleepingAfterThinking= false;
	sleeping= true;

	wakeUpTimer= 0.0;

	wakeUpIn->setOnReceiveCallback([&] ()
	{
		thinkingTimeout= 0.0;
		sleeping= false;
		wakeUpTimer= timeAwakeIn->get(); // Start sleeping when this goes to zero
	});

	setUpdateNeeded(true);
}

void RollBotAiNodeInstance::update()
{
	real32 dt= global::g_env.worldMgr->getDeltaTime();

	if ( abs(escapeIn->get().x - positionIn->get().x) < 20 ) { // Escape if too close
		if (escapeIn->get().x > positionIn->get().x)
			wantedDirection= -1;
		else
			wantedDirection= 1;
	} else {
		if (wantedDirection == 0) { // Random wandering
			wantedDirection= util::Rand::discrete(0, 1)*2 - 1;
		}
	}
	wakeUpTimer -= dt;

	if (wakeUpTimer < 0.0) {
		thinkingTimeout= 0.0;
		sleeping= true;
	}

	bool thinking= false;
	if (thinkingTimeout > 0.001)
		thinking= true;

	if (util::Rand::continuous(0.0, 1.0) < thinkingProbability*dt){
		// Start thinking
		thinkingProbability= 0.0;
		thinkingTimeout= util::Rand::continuous(1.0, 3.0);

		//if (thinkingTimeout > 5.0 && randInt(0,5) == 0)
		startSleepingAfterThinking= true;
	}

	real64 target_omega= 0;
	real64 max_torque= 1.0;

	if (!thinking && !sleeping) {
		target_omega= -15.0*wantedDirection;

		if (abs(lastOmega) > abs(omegaIn->get())) {
			// Slowing down
			frustration += 0.1*std::min(std::abs(lastOmega - omegaIn->get()), 1.0);
		}

		if (abs(omegaIn->get()) < 0.1*abs(target_omega)) {
			// Too slow
			frustration += 5.0*(0.1-abs(omegaIn->get()))*dt;	
		}

		frustration -= 0.05*dt;
		frustration= std::max(frustration, 0.0f);

		if (frustration > 1.0) {
			wantedDirection= -wantedDirection;
			frustration *= 0.5;
		}

		hardPushTimeout= std::max(hardPushTimeout - dt, 0.0f);

		if (abs(omegaIn->get()) < 0.1 && frustration > 0.8 && hardPushTimeout < 0.001) {
			// More torque if stuck
			max_torque= 2.5;
			hardPushTimeout= 1.0;
		}

		thinkingProbability= (thinkingProbability + 0.1*dt)/(1 + (thinkingProbability*0.1*dt));
	} else {
		// Thinking or sleeping, stay still
		max_torque= 0.3;
		wantedDirection= 0;
		frustration= 0.0;
		thinkingTimeout= thinkingTimeout - dt;

		if (thinkingTimeout < 0.0) {
			thinkingTimeout= 0.0;
			if (startSleepingAfterThinking) {
				sleeping= true;
				startSleepingAfterThinking= false;
			}
		}
	}

	// Moving

	real64 torque= (target_omega - omegaIn->get())*(1.0 + frustration*2.0);
	torque= util::limited(torque, -max_torque, max_torque);

	if (	(torque > 0 && wantedDirection > 0) ||
			(torque < 0 && wantedDirection < 0)) {
		// Roll freely at downhill
		torque= 0;
	}

	awakeOut->send(!sleeping);
	torqueOut->send(torque*torqueMulIn->get());

	lastOmega= omegaIn->get();
}

} // nodes
} // clover
