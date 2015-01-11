#ifndef CLOVER_NODES_NODEINSTANCE_MULTIPLY_REAL_HPP
#define CLOVER_NODES_NODEINSTANCE_MULTIPLY_REAL_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class RealMultiplyNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;

private:
	InputSlot<SignalType::Real>* input1;
	InputSlot<SignalType::Real>* input2;
	OutputSlot<SignalType::Real>* realOut;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::RealMultiplyNodeInstance> {
	static util::Str8 type(){ return "::RealMultiplyNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_MULTIPLY_REAL_HPP
