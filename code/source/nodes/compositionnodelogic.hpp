#ifndef CLOVER_NODES_COMPOSITIONNODELOGIC_HPP
#define CLOVER_NODES_COMPOSITIONNODELOGIC_HPP

#include "build.hpp"
#include "compositionnodeslot.hpp"
#include "compositionnodeslottemplategroup.hpp"
#include "resources/cache.hpp"
#include "resources/resource.hpp"
#include "slotidentifier.hpp"
#include "util/cb_listener.hpp"
#include "util/hashmap.hpp"

#include <memory>

namespace clover {
namespace nodes {
class NodeType;

class CompositionNodeLogicGroup;

/// Contains settings for a NodeType which'll be instantiated
class CompositionNodeLogic : public util::Callbacker<util::OnChangeCb> {
public: // Copypasted from former BaseCompositionNodeScriptLogic

	typedef std::unique_ptr<CompositionNodeSlot> CompositionNodeSlotPtr;
	typedef std::unique_ptr<CompositionNodeSlotTemplateGroup> CompositionNodeSlotTemplateGroupPtr;
	
	CompositionNodeSlot& addInputSlot(const SlotIdentifier& identifier, const boost::any& init_value);
	
	template <typename T>
	CompositionNodeSlot& addInputSlot(const util::Str8& name, const SignalType& signal_type, const T& init_value){
		return addInputSlot(name, "", signal_type, init_value);
	}
	
	template <typename T>
	CompositionNodeSlot& addInputSlot(const util::Str8& name, const util::Str8& group, const SignalType& signal_type, const T& init_value){
		return addInputSlot(SlotIdentifier{name, group, signal_type, true}, boost::any(init_value));
	}
	
	CompositionNodeSlot& addInputSlot(const util::Str8& name, const SignalType& signal_type){
		return addInputSlot(name, "", signal_type);
	}
	
	CompositionNodeSlot& addInputSlot(const SlotIdentifier& id){
		return addInputSlot(id.name, id.groupName, id.signalType);
	}
	
	CompositionNodeSlot& addInputSlot(const util::Str8& name, const util::Str8& group, const SignalType& signal_type){
		return addInputSlot(SlotIdentifier{name, group, signal_type, true}, RuntimeSignalTypeTraits::defaultInitValue(signal_type));
	}
	
	CompositionNodeSlot& addOutputSlot(const SlotIdentifier& identifier);

	CompositionNodeSlot& addOutputSlot(const util::Str8& name, const SignalType& signal_type){
		return addOutputSlot(name, "", signal_type);
	}
	
	CompositionNodeSlot& addOutputSlot(const util::Str8& name, const util::Str8& group, const SignalType& signal_type){
		return addOutputSlot(SlotIdentifier{name, group, signal_type, false});
	}

	CompositionNodeSlot& addSlot(const SlotIdentifier& id, boost::any init_value);
	CompositionNodeSlot& addSlot(const SlotIdentifier& id);
	
	void removeSlot(const SlotIdentifier& id);
	void removeSlot(const CompositionNodeSlot& slot){ removeSlot(slot.getIdentifier()); }
	
	CompositionNodeSlot& getSlot(const SlotIdentifier& id) const;
	bool hasSlot(const SlotIdentifier& id) const { return slots.find(id) != slots.end(); }
	
	const util::DynArray<CompositionNodeSlot*>& getSlots() const { return slotArray; }
	util::DynArray<CompositionNodeSlot*>& getSlots() { return slotArray; }
	
	util::DynArray<SlotIdentifier> getSlotIdentifiers() const;
	
	CompositionNodeSlotTemplateGroup& addSlotTemplateGroup(const util::Str8& name, bool input);
	CompositionNodeSlotTemplateGroup& addInputSlotTemplateGroup(const util::Str8& name){ return addSlotTemplateGroup(name, true); }
	CompositionNodeSlotTemplateGroup& addOutputSlotTemplateGroup(const util::Str8& name){ return addSlotTemplateGroup(name, false); }
	
	const CompositionNodeSlotTemplateGroup& getSlotTemplateGroup(const util::Str8& name) const;
	CompositionNodeSlotTemplateGroup& getSlotTemplateGroup(const util::Str8& name);
	
	bool hasSlotTemplateGroup(const util::Str8& name) const;
	
	/// First to update in a NodeGroup
	void setAsUpdateRouteStart(bool b);
	bool isUpdateRouteStart() const { return updateRouteStart; }
	
	template <typename T>
	void addResourceChangeListener(const typename resources::ResourceTraits<T>::IdentifierValue& identifier){
		const auto& res= global::g_env->resCache->getResource<T>(identifier);
		addOnResourceChangeCallback(res);
	}
	void clearResourceChangeListeners(){ resourceChangeListener.clear(); }

	void setBatched(bool b= true){ batched= b; }
	void setBatchPriority(int32 p){ batchPriority= p; }
	
	bool isBatched() const { return batched; }
	int32 getBatchPriority() const { return batchPriority; }

	const util::HashMap<util::Str8, CompositionNodeSlotTemplateGroupPtr>& getSlotTemplateGroups() const { return templateGroups; }

	virtual void onDefaultValueChange(CompositionNodeSlot& slot);
	void onRoutingChange(CompositionNodeSlot& slot);
	virtual void onResourceChange(const resources::Resource& res) {}
	
	void clear();
	
	// Doesn't send events if silent
	void setSilent(bool b= true){ silent= b; }
	
private:
	// Just adds a new slot to the containers
	CompositionNodeSlot& addSlotMinimal(const SlotIdentifier& id);
	
	void sendOnSlotAddEvent(const SlotIdentifier& id);
	void sendOnSlotRemoveEvent(const SlotIdentifier& id);
	
	void addOnResourceChangeCallback(const resources::Resource& res);

	bool batched;
	int32 batchPriority;

	util::HashMap<SlotIdentifier, CompositionNodeSlotPtr> slots;
	util::DynArray<CompositionNodeSlot*> slotArray; // To preserve ordering
	util::HashMap<util::Str8, CompositionNodeSlotTemplateGroupPtr> templateGroups;
	
	bool updateRouteStart;
	bool silent; // Don't emit any events (doesn't probably cover all cases, is used when slot is replaced by changed one)
	util::CbMultiListener<util::OnChangeCb> resourceChangeListener;




public: // Original CompositionNodeLogic
	CompositionNodeLogic();
	void setType(const NodeType& type_);
	CompositionNodeLogic(CompositionNodeLogic&&)= delete;
	
	void setOwner(const CompositionNodeLogicGroup& o){ owner= &o; }
	const CompositionNodeLogicGroup& getOwner() const { ensure(owner); return *owner; }
	
	CompositionNodeSlot& getInputSlot(const SlotIdentifier& id) const;
	CompositionNodeSlot& getOutputSlot(const SlotIdentifier& id) const;
	
	const NodeType& getType() const;
	
	util::DynArray<CompositionNodeSlot*> getTemplateGroupSlots() const;

	util::DynArray<CompositionNodeSlotTemplateGroup*> getInputSlotTemplateGroups() const;
	util::DynArray<CompositionNodeSlotTemplateGroup*> getOutputSlotTemplateGroups() const;
	util::DynArray<CompositionNodeSlotTemplateGroup*> getSlotTemplateGroups(bool input) const;
	
	void addSlot(const SlotTemplate& slot);
	void removeSlot(const SlotTemplate& slot);
	
	void setPosition(const util::Vec2d& p){ position= p; }
	const util::Vec2d& getPosition() const { return position; }
	
private:
	void recreate();
	
	const NodeType* type;
	const CompositionNodeLogicGroup* owner;

	util::Vec2d position;
	util::CbListener<util::OnChangeCb> typeChangeListener;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::CompositionNodeLogic> {
	static util::Str8 type(){ return "::CompositionNodeLogic"; } 
};

} // util
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODELOGIC_HPP
