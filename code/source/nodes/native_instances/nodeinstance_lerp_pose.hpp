#ifndef CLOVER_NODES_NODEINSTANCE_LERP_POSE_HPP
#define CLOVER_NODES_NODEINSTANCE_LERP_POSE_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class PoseLerpNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual ~PoseLerpNodeInstance(){}
	
	virtual void create() override;
	virtual void update() override;
	
private:
	InputSlot<SignalType::ArmaturePose>* input1;
	InputSlot<SignalType::ArmaturePose>* input2;
	InputSlot<SignalType::Real>* factorIn;
	InputSlot<SignalType::Boolean>* limitFactorIn;
	
	OutputSlot<SignalType::ArmaturePose>* resultOut;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::PoseLerpNodeInstance> {
	static util::Str8 type(){ return "::PoseLerpNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_LERP_POSE_HPP
