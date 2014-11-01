#ifndef CLOVER_NODES_NODEINSTANCE_WE_SPATIALEVENTDISPATCHER_HPP
#define CLOVER_NODES_NODEINSTANCE_WE_SPATIALEVENTDISPATCHER_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class NodeEventType;

class WeSpatialEventDispatcherNodeInstance : public NodeInstance {
public:

	virtual ~WeSpatialEventDispatcherNodeInstance(){}

	virtual void create();
	virtual void update();

private:
	
	InputSlot<SignalType::SrtTransform2>* transformInput;
	InputSlot<SignalType::Event>* eventInput;
	InputSlot<SignalType::Real>* radiusInput;
	
	OutputSlot<SignalType::Event>* forwardOutput;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeSpatialEventDispatcherNodeInstance> {
	static util::Str8 type(){ return "::WeSpatialEventDispatcherNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WE_SPATIALEVENTDISPATCHER_HPP
