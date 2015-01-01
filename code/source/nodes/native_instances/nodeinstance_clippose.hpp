#ifndef CLOVER_NODES_NODEINSTANCE_CLIPPOSE_HPP
#define CLOVER_NODES_NODEINSTANCE_CLIPPOSE_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace animation {

class Clip;

} // animation
namespace nodes {

class ClipPoseNodeInstance : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();
	
	virtual ~ClipPoseNodeInstance(){}
	
	virtual void create() override;
	virtual void update() override;
	
private:
	InputSlot<SignalType::String>* clipIn;
	InputSlot<SignalType::Boolean>* interpolateIn;
	InputSlot<SignalType::Real>* phaseIn;
	
	OutputSlot<SignalType::ArmaturePose>* poseOut;
	
	const animation::Clip* clip;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::ClipPoseNodeInstance> {
	static util::Str8 type(){ return "::ClipPoseNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_CLIPPOSE_HPP
