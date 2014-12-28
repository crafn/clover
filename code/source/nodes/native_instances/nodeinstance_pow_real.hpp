#ifndef CLOVER_NODES_NODEINSTANCE_POW_REAL_HPP
#define CLOVER_NODES_NODEINSTANCE_POW_REAL_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class RealPowNodeInstance : public NodeInstance {
public:
	virtual void create() override;

private:
	InputSlot<SignalType::Real>* baseInput;
	InputSlot<SignalType::Real>* exponentInput;
	OutputSlot<SignalType::Real>* realOut;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::RealPowNodeInstance> {
	static util::Str8 type(){ return "::RealPowNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_POW_REAL_HPP
