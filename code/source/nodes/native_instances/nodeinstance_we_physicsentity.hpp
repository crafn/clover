#ifndef CLOVER_NODES_NODEINSTANCE_WE_PHYSICSENTITY_HPP
#define CLOVER_NODES_NODEINSTANCE_WE_PHYSICSENTITY_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "physics/entity.hpp"

namespace clover {
namespace nodes {

class WePhysicsEntityNodeInstance : public NodeInstance {
public:
	virtual ~WePhysicsEntityNodeInstance(){}
	
	virtual void create();
	virtual void update();
	
private:
	void recreateEntity();

	InputSlot<SignalType::Boolean>* activeInput;
	InputSlot<SignalType::SrtTransform3>* transformInput;
	InputSlot<SignalType::String>* entityDefInput;
	InputSlot<SignalType::WeHandle>* weInput;
	
	OutputSlot<SignalType::SrtTransform3>* transformOutput;
	OutputSlot<SignalType::SrtTransform3>* estimatedTransformOutput;
	OutputSlot<SignalType::ArmaturePose>* poseOutput;
	
	physics::Entity entity;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WePhysicsEntityNodeInstance> {
	static util::Str8 type(){ return "::WePhysicsEntityNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WE_PHYSICSENTITY_HPP
