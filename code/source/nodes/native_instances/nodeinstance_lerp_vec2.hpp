#ifndef CLOVER_NODES_NODEINSTANCE_LERP_VEC2_HPP
#define CLOVER_NODES_NODEINSTANCE_LERP_VEC2_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class Vec2LerpNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::Vec2>* in1;
	InputSlot<SignalType::Vec2>* in2;
	InputSlot<SignalType::Real>* factorIn;
	OutputSlot<SignalType::Vec2>* resultOut;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::Vec2LerpNodeInstance> {
	static util::Str8 type(){ return "::Vec2LerpNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_LERP_VEC2_HPP
