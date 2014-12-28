#ifndef CLOVER_NODES_NODEINSTANCE_DELAYEDTRIGGER_HPP
#define CLOVER_NODES_NODEINSTANCE_DELAYEDTRIGGER_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class DelayedTriggerNodeInstance : public NodeInstance {
public:
	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::Trigger>* triggerIn;
	InputSlot<SignalType::Real>* timeIn;

	OutputSlot<SignalType::Trigger>* triggerOut;

	real64 timer;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::DelayedTriggerNodeInstance> {
	static util::Str8 type(){ return "::DelayedTriggerNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_DELAYEDTRIGGER_HPP
