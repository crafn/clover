#ifndef CLOVER_NODES_NODEINSTANCE_EVENTFACTORY_HPP
#define CLOVER_NODES_NODEINSTANCE_EVENTFACTORY_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class NodeEventType;

class EventFactoryNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual ~EventFactoryNodeInstance(){}

	virtual void create();
	virtual void update();

private:
	InputSlot<SignalType::EventType>* eventTypeInput;
	InputSlot<SignalType::Trigger>* triggerInput;
	OutputSlot<SignalType::Event>* eventOutput;
	
	const NodeEventType* eventType;
	
	util::HashMap<util::Str8, BaseInputSlot*> argumentInputs;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::EventFactoryNodeInstance> {
	static util::Str8 type(){ return "::EventFactoryNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_EVENTFACTORY_HPP
