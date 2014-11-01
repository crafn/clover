#ifndef CLOVER_NODES_NODEINSTANCE_WE_PHYSICSOBJECT_HPP
#define CLOVER_NODES_NODEINSTANCE_WE_PHYSICSOBJECT_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "physics/object_rigid.hpp"

namespace clover {
namespace nodes {

class WePhysicsObjectNodeInstance : public NodeInstance {
public:

	virtual ~WePhysicsObjectNodeInstance(){}
	
	virtual void create();
	virtual void update();
	
private:
	void recreateObject();
	void sendShape();
	
	InputSlot<SignalType::Boolean>* activeInput;
	InputSlot<SignalType::RtTransform2>* forceInput;
	InputSlot<SignalType::SrtTransform3>* transformInput;
	InputSlot<SignalType::RtTransform2>* impulseInput;
	InputSlot<SignalType::Shape>* shapeInput;
	InputSlot<SignalType::String>* materialInput;
	InputSlot<SignalType::Boolean>* staticInput;
	InputSlot<SignalType::Event>* eventInput;
	InputSlot<SignalType::Boolean>* partialBreakingInput;
	InputSlot<SignalType::WeHandle>* weInput;
	
	OutputSlot<SignalType::SrtTransform3>* transformOutput;
	OutputSlot<SignalType::SrtTransform3>* estimatedTransformOutput;
	OutputSlot<SignalType::RtTransform2>* velocityOutput;
	OutputSlot<SignalType::RtTransform2>* impulseOutput;
	OutputSlot<SignalType::RtTransform2>* accelerationOutput;
	OutputSlot<SignalType::Shape>* onShapeChangeOutput;
	OutputSlot<SignalType::Trigger>* onBreakOutput;
	
	util::CbListener<physics::OnBreakCb> breakListener;
	
	util::RtTransform2d lastVelocity; // Used only when impulseOutput or accelerationOutput is connected
	// Must instantiate dynamically for pooling to take place
	util::UniquePtr<physics::RigidObject> object;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WePhysicsObjectNodeInstance> {
	static util::Str8 type(){ return "::WePhysicsObjectNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WE_PHYSICSOBJECT_HPP
