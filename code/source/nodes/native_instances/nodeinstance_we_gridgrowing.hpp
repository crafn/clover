#ifndef CLOVER_NODES_NATIVE_INSTANCE_NODEINSTANCE_WE_GRIDGROWING_HPP
#define CLOVER_NODES_NATIVE_INSTANCE_NODEINSTANCE_WE_GRIDGROWING_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class WeGridGrowingNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::RtTransform2>* transformIn;
	InputSlot<SignalType::Real>* growRateIn;
	InputSlot<SignalType::Real>* decayRateIn;
	InputSlot<SignalType::Trigger>* checkIn;

	OutputSlot<SignalType::Real>* stateOut;

	real64 state= 0.0;
	real64 stateDeriv= 0.0;
	bool grow= false;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeGridGrowingNodeInstance> {
	static util::Str8 type(){ return "::WeGridGrowingNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NATIVE_INSTANCE_NODEINSTANCE_WE_GRIDGROWING_HPP
