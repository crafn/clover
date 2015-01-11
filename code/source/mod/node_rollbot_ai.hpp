#ifndef CLOVER_NODES_NODEINSTANCE_ROLLBOT_AI_HPP
#define CLOVER_NODES_NODEINSTANCE_ROLLBOT_AI_HPP

#include "nodes/nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace mod {

// I'm lazy
using namespace clover::nodes;

DECLARE_NODE(RollBotAiNode);
class RollBotAiNode final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	void update_novirtual();

private:
public:
	InputSlot<SignalType::RtTransform2>* aIn;
	InputSlot<SignalType::Real>* omegaIn;
	InputSlot<SignalType::Real>* torqueMulIn;
	InputSlot<SignalType::Trigger>* wakeUpIn;
	InputSlot<SignalType::Real>* timeAwakeIn;
	InputSlot<SignalType::Vec2>* escapeIn;
	InputSlot<SignalType::Vec2>* positionIn;

	OutputSlot<SignalType::Real>* torqueOut;
	OutputSlot<SignalType::Boolean>* awakeOut;

	int32 wantedDirection;
	real32 frustration;
	real32 lastOmega;
	real32 hardPushTimeout;
	real32 thinkingTimeout;
	real32 thinkingProbability;
	real32 wakeUpTimer;
	bool startSleepingAfterThinking;
	bool sleeping;
};

} // mod
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_ROLLBOT_AI_HPP
