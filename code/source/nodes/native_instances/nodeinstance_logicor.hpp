#ifndef CLOVER_NODES_NODEINSTANCE_LOGICOR_HPP
#define CLOVER_NODES_NODEINSTANCE_LOGICOR_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

class LogicOrNodeInstance final : public NodeInstance {
public:
	static CompositionNodeLogic* compNode();

	virtual ~LogicOrNodeInstance(){}
	
	virtual void create();
	virtual void update();
	
private:
	InputSlot<SignalType::Boolean>* input1;
	InputSlot<SignalType::Boolean>* input2;
	OutputSlot<SignalType::Boolean>* output;

};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::LogicOrNodeInstance> {
	static util::Str8 type(){ return "::LogicOrNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_LOGICOR_HPP
