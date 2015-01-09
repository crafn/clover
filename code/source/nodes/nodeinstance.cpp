#include "nodeinstance.hpp"
#include "util/profiling.hpp"
#include "compositionnodelogic.hpp"

// For dump
#include "nodetype.hpp"
#include "compositionnodelogicgroup.hpp"

namespace clover {
namespace nodes {

NodeInstance::NodeInstance()
		: compositionNodeLogic(nullptr)
		, type(nullptr)
		, groupVars(nullptr)
		, updateNeeded(false)
{ }

NodeInstance::~NodeInstance()
{ }

void NodeInstance::setCompositionNodeLogic(const CompositionNodeLogic& comp)
{ compositionNodeLogic= &comp; }

const CompositionNodeLogic& NodeInstance::getCompositionNodeLogic() const
{ return *NONULL(compositionNodeLogic); }

void NodeInstance::receiveSignals()
{
	PROFILE();
	for (auto& m : inputSlots) {
		m.slot->update(*this);
	}
}

void NodeInstance::baseUpdate()
{
	PROFILE();

	ensure(groupVars);
	ensure(groupVars->firstOfGroup);

	if (groupVars->firstOfGroup == this) {
		groupVars->signalsSent= false;
	} else {
		receiveSignals();
	}

	if (isUpdateNeeded()) {
		PROFILE();
		type->updateInstance(*this);
	}

	if (groupVars->lastOfGroup == this)
		groupVars->firstOfGroup->receiveSignals();
}

BaseInputSlot* NodeInstance::getInputSlot(const SlotIdentifier& id) const
{
	for (auto& m : inputSlots) {
		if (util::hash32(id) == m.hash) return m.slot.get();
	}
	return nullptr;
}

BaseOutputSlot* NodeInstance::getOutputSlot(const SlotIdentifier& id) const
{
	for (auto& m : outputSlots) {
		if (util::hash32(id) == m.hash) return m.slot.get();
	}
	return nullptr;
}

BaseInputSlot* NodeInstance::addInputSlot(const util::Str8& name, const util::Str8& group, SignalType s)
{
	#define SIGNAL(x, n) \
		if (s == SignalType::x){ return addInputSlot<SignalType::x>(name, group); }
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("NodeInstance::addInputSlot(..): Invalid SignalType %i, %s", s, name.cStr());
}

BaseOutputSlot* NodeInstance::addOutputSlot(const util::Str8& name, const util::Str8& group, SignalType s)
{
	#define SIGNAL(x, n) \
		if (s == SignalType::x){ return addOutputSlot<SignalType::x>(name, group); }
	#include "signaltypes.def"
	#undef SIGNAL

	throw global::Exception("NodeInstance::addOutputSlot(..): Invalid SignalType %i, %s", s, name.cStr());		
}

bool NodeInstance::isBatched() const
{ return getCompositionNodeLogic().isBatched(); }

int32 NodeInstance::getBatchPriority() const
{ return getCompositionNodeLogic().getBatchPriority(); }

util::Str8 NodeInstance::identityDump() const
{
	return util::Str8::format("%s in %s",
		getType().getName().cStr(),
		getCompositionNodeLogic().getOwner().getName().cStr());	
}

void NodeInstance::setUpdateNeeded(bool b)
{
	int32 dif= (int32)b - (int32)updateNeeded;
	NONULL(groupVars)->updatesNeeded += dif;

	updateNeeded= b;
}

} // nodes
} // clover
