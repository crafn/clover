#ifndef CLOVER_NODES_BASEOUTPUTSLOT_HPP
#define CLOVER_NODES_BASEOUTPUTSLOT_HPP

#include "build.hpp"
#include "baseslot.hpp"
#include "util/dyn_array.hpp"

namespace clover {
namespace nodes {

class BaseInputSlot;

template <SignalType S>
class OutputSlot;

class BaseOutputSlot : public BaseSlot {
public:
	BaseOutputSlot(SignalType t);
	virtual ~BaseOutputSlot();

	void attach(SubSignalType from, SubSignalType to, BaseInputSlot& input);
	bool isAttached() const { return !inputs.empty(); }

	virtual void send(const boost::any& value) const = 0;

	template <SignalType S>
	OutputSlot<S>& casted(){
		ensure(S == getType());
		return *static_cast<OutputSlot<S>*>(this);
	}

protected:
	struct InputSlotInfo {
		BaseInputSlot* slot;
		SubSignalType fromSub; // If == SubSignalType::None, normal signalling between slots
		SubSignalType toSub;
	};

	util::DynArray<InputSlotInfo> inputs;
};

} // nodes
} // clover

#endif // CLOVER_NODES_BASEOUTPUTSLOT_HPP
