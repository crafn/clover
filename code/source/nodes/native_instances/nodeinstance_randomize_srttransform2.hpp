#ifndef CLOVER_NODES_NODEINSTANCE_RANDOMIZE_SRTTRANSFORM2_HPP
#define CLOVER_NODES_NODEINSTANCE_RANDOMIZE_SRTTRANSFORM2_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class SrtTransform2RandomizeNodeInstance : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::SrtTransform2>* transformIn;
	InputSlot<SignalType::Real>* rotBiasIn;
	InputSlot<SignalType::Real>* scaleBiasIn;
	InputSlot<SignalType::Boolean>* flipXIn;

	OutputSlot<SignalType::SrtTransform2>* transformOut;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::SrtTransform2RandomizeNodeInstance> {
	static util::Str8 type(){ return "::SrtTransform2RandomizeNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_RANDOMIZE_SRTTRANSFORM2_HPP
