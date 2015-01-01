#ifndef CLOVER_NODES_NODEINSTANCE_EXP_RELEASE_REAL_HPP
#define CLOVER_NODES_NODEINSTANCE_EXP_RELEASE_REAL_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class RealExpReleaseNodeInstance : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::Real>* realLimitIn;
	InputSlot<SignalType::Real>* accumIn;
	InputSlot<SignalType::Real>* targetIn;
	InputSlot<SignalType::Real>* halfValueTime;

	OutputSlot<SignalType::Real>* valueOut;

	real64 value;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::RealExpReleaseNodeInstance> {
	static util::Str8 type(){ return "::RealExpReleaseNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_EXP_RELEASE_REAL
