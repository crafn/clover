#ifndef CLOVER_NODES_NODEINSTANCE_EVENTHANDLER_HPP
#define CLOVER_NODES_NODEINSTANCE_EVENTHANDLER_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class NodeEventType;

class EventHandlerNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual ~EventHandlerNodeInstance(){}

	virtual void create();
	virtual void update();

private:
	InputSlot<SignalType::EventType>* eventTypeInput;
	InputSlot<SignalType::EventArray>* eventsInput;

	OutputSlot<SignalType::Event>* forwardOutput;
	
	const NodeEventType* eventType;
	SignalTypeTraits<SignalType::EventArray>::Value events;
	util::HashMap<util::Str8, BaseOutputSlot*> argumentOutputs;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::EventHandlerNodeInstance> {
	static util::Str8 type(){ return "::EventHandlerNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_EVENTHANDLER_HPP
