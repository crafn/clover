#ifndef CLOVER_NODES_NODEINSTANCE_ROLLBOT_AI_HPP
#define CLOVER_NODES_NODEINSTANCE_ROLLBOT_AI_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class RollBotAiNodeInstance : public NodeInstance {
public:
	virtual void create() override;
	virtual void update() override;

private:
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

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::RollBotAiNodeInstance> {
	static util::Str8 type(){ return "::RollBotAiNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_ROLLBOT_AI_HPP
