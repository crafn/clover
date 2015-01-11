#ifndef CLOVER_NODES_NODEINSTANCE_TRIGGERTOSIGNAL_HPP
#define CLOVER_NODES_NODEINSTANCE_TRIGGERTOSIGNAL_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class TriggerToSignalNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::Trigger>* triggerIn;
	InputSlot<SignalType::Real>* onValueIn;
	InputSlot<SignalType::Real>* offValueIn;

	OutputSlot<SignalType::Real>* signalOut;

	bool triggerReceived;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::TriggerToSignalNodeInstance> {
	static util::Str8 type(){ return "::TriggerToSignalNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_TRIGGERTOSIGNAL_HPP
