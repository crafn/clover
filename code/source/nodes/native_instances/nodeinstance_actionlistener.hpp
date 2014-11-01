#ifndef CLOVER_NODES_NODEINSTANCE_ACTIONLISTENER_HPP
#define CLOVER_NODES_NODEINSTANCE_ACTIONLISTENER_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "ui/hid/actionlistener.hpp"
#include "util/map.hpp"

/// @todo Replace with util
#include <boost/optional.hpp>

namespace clover {
namespace nodes {

class ActionListenerNodeInstance : public NodeInstance {
public:
	virtual ~ActionListenerNodeInstance(){}
	virtual void create();

private:
	InputSlot<SignalType::String>* channelNameIn;
	InputSlot<SignalType::String>* tagIn;
	InputSlot<SignalType::String>* actionNameIn;
	OutputSlot<SignalType::Trigger>* onActionOut;
	util::Map<SignalType, BaseOutputSlot*> valueOutputs;
	
	boost::optional<ui::hid::ActionListener<>> actionListener; 
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::ActionListenerNodeInstance> {
	static util::Str8 type(){ return "::ActionListenerNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_ACTIONLISTENER_HPP
