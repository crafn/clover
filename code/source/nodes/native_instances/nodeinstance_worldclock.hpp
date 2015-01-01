#ifndef CLOVER_NODES_NODEINSTANCE_WORLDCLOCK_HPP
#define CLOVER_NODES_NODEINSTANCE_WORLDCLOCK_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class WorldClockNodeInstance : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	OutputSlot<SignalType::Real>* dayPhaseOut;
	OutputSlot<SignalType::Real>* daynessOut;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WorldClockNodeInstance> {
	static util::Str8 type(){ return "::WorldClockNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WORLDCLOCK_HPP
