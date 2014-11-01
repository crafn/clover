#ifndef CLOVER_NODES_NODEINSTANCELOGIC_HPP
#define CLOVER_NODES_NODEINSTANCELOGIC_HPP

#include "build.hpp"
#include "compositionnodelogic.hpp"
#include "compositionnodeslot.hpp"
#include "signaltypetraits.hpp"
#include "inputslot.hpp"
#include "outputslot.hpp"
#include "util/linkedlist.hpp"

#include <memory>

namespace clover {
namespace nodes {

class BaseInputSlot;
class BaseOutputSlot;
class CompositionNodeSlot;
class NodeType;

class NodeInstance {
	typedef std::unique_ptr<BaseInputSlot> InputSlotPtr;
	typedef std::unique_ptr<BaseOutputSlot> OutputSlotPtr;
public:
	// External variables stored in NodeInstanceGroup
	// Used for optimization and group behavior
	struct GroupVars {
		// +1 when starts needing an update, -1 when stops
		int updatesNeeded= 0;

		// InputSlot sets to true when receives a signal
		// Reset by first node of the group
		bool signalsSent= false;

		// First node to be updated in a NodeInstanceGroup
		// Used to detect when to reset external variables
		NodeInstance* firstOfGroup= nullptr;
		NodeInstance* lastOfGroup= nullptr;
	};

	NodeInstance();
	virtual ~NodeInstance();
	
	void setCompositionNodeLogic(const CompositionNodeLogic& comp);
	const CompositionNodeLogic& getCompositionNodeLogic() const;
	
	void setType(const NodeType& type_){ type= &type_; }
	const NodeType& getType() const { ensure(type); return *type; }
	
	virtual void create(){}
	
	/// Called after input signals are received and before output signals are sent
	/// Used to implement custom logic in derived types. Use fullUpdate()
	/// to update a node
	virtual void update(){}

	void receiveSignals();

	/// Receives signals and calls update() _if_needed_ !
	/// This is the only method needed to call when updating a group
	void baseUpdate();

	bool isUpdateNeeded() const { return updateNeeded; }

	BaseInputSlot* getInputSlot(const SlotIdentifier& id) const;
	BaseOutputSlot* getOutputSlot(const SlotIdentifier& id) const;

	/// Called only by derived types
	template <SignalType S>
	InputSlot<S>* addInputSlot(const util::Str8& name, const util::Str8& group= ""){
		auto ptr= new InputSlot<S>();
		auto id= SlotIdentifier{name, group, S, true};
		inputSlots.pushBack(SlotWrap<InputSlotPtr>(id, InputSlotPtr(ptr)));
		setInputSlotValueToDefault<S>(id, *inputSlots.back().slot);
		return ptr;
	}

	template <SignalType S>
	InputSlot<S>* addInputSlot(const CompositionNodeSlot& slot){
		auto ptr= new InputSlot<S>();
		inputSlots.pushBack(SlotWrap<InputSlotPtr>(slot.getIdentifier(), InputSlotPtr(ptr)));
		setInputSlotValueToDefault<S>(slot.getIdentifier(), *inputSlots.back().slot);
		return ptr;
	}

	template <SignalType S>
	OutputSlot<S>* addOutputSlot(const util::Str8& name, const util::Str8& group= ""){
		auto ptr= new OutputSlot<S>();
		outputSlots.pushBack(SlotWrap<OutputSlotPtr>(SlotIdentifier{name, group, S, false}, OutputSlotPtr(ptr)));
		return ptr;
	}

	template <SignalType S>
	OutputSlot<S>* addOutputSlot(const CompositionNodeSlot& slot){
		auto ptr= new OutputSlot<S>();
		outputSlots.pushBack(SlotWrap<OutputSlotPtr>(slot.getIdentifier(), OutputSlotPtr(ptr)));
		return ptr;
	}

	BaseInputSlot* addInputSlot(const util::Str8& name, SignalType s){ return addInputSlot(name, "", s); }
	BaseInputSlot* addInputSlot(const util::Str8& name, const util::Str8& group, SignalType s);
	BaseOutputSlot* addOutputSlot(const util::Str8& name, SignalType s){ return addOutputSlot(name, "", s); }
	BaseOutputSlot* addOutputSlot(const util::Str8& name, const util::Str8& group, SignalType s);
	
	SizeType getInputSlotCount() const { return inputSlots.size(); }
	SizeType getOutputSlotCount() const { return outputSlots.size(); }
	
	void setGroupVars(GroupVars* g){ groupVars= g; }

	bool isBatched() const;
	int32 getBatchPriority() const;

	util::Str8 identityDump() const;

protected:
	void setUpdateNeeded(bool b= true);
	
	template <typename Ptr>
	struct SlotWrap {
		SlotWrap(const SlotIdentifier& id, Ptr&& p):
			hash(util::hash32(id)),
			slot(std::move(p)){
		}

		uint32 hash;
		Ptr slot;
	};

	const util::DynArray<SlotWrap<InputSlotPtr>>& getInputSlots() const { return inputSlots; }
	const util::DynArray<SlotWrap<OutputSlotPtr>>& getOutputSlots() const { return outputSlots; }
	
private:
	template <SignalType S>
	void setInputSlotValueToDefault(const SlotIdentifier& identifier, BaseInputSlot& input_slot){
		const CompositionNodeSlot& comp_slot= compositionNodeLogic->getInputSlot(identifier);
		input_slot.setDefaultValue(comp_slot.getDefaultValue());
		
		// If default value is set by user, send it as a signal in the first frame
		// If default value is not set by user, comp_slot->getDefaultValue() is just an initialization value and is not sent
		if (comp_slot.isDefaultValueSet())
			input_slot.setValueReceived();
	}

protected:
	const CompositionNodeLogic* compositionNodeLogic;
	
private:
	const NodeType* type;
	GroupVars* groupVars;

	// Identifier hash, pointer to slot
	util::DynArray<SlotWrap<InputSlotPtr>> inputSlots;
	util::DynArray<SlotWrap<OutputSlotPtr>> outputSlots;
	
	bool updateNeeded;
};

} // nodes
} // clover

#endif // CLOVER_NODES_NODEINSTANCELOGIC_HPP