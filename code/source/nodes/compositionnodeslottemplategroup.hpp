#ifndef CLOVER_NODES_COMPOSITIONNODESLOTTEMPLATEGROUP_HPP
#define CLOVER_NODES_COMPOSITIONNODESLOTTEMPLATEGROUP_HPP

#include "build.hpp"
#include "compositionnodeslottemplategrouphandle.hpp"
#include "script/reference.hpp"
#include "signaltypetraits.hpp"
#include "slotidentifier.hpp"

namespace clover {
namespace nodes {

class CompositionNodeLogic;
class CompositionNodeSlotTemplateGroup;

class SlotTemplate {
public:
	SlotTemplate(CompositionNodeSlotTemplateGroup& owner_, const util::Str8& name_, SignalType type, bool is_input)
		: owner(&owner_)
		, name(name_)
		, signalType(type)
		, input(is_input){}

	void setOwner(CompositionNodeSlotTemplateGroup& owner_){ owner= &owner_; }
	CompositionNodeSlotTemplateGroup& getOwner() const { return *owner.get(); }
	const util::Str8& getName() const { return name; }
	SignalType getSignalType() const { return signalType; }
	SlotIdentifier getIdentifier() const;
	bool isInput() const { return input; }
	boost::any getInitValue() const { return RuntimeSignalTypeTraits::defaultInitValue(signalType); }
	SlotTemplate mirrored() const { SlotTemplate s(*this); s.input= !s.input; return s; }
	
private:
	CompositionNodeSlotTemplateGroupHandle owner;
	util::Str8 name;
	SignalType signalType;
	bool input;
};

/// Rules for a dynamic group of slots
class CompositionNodeSlotTemplateGroup : public script::NoCountReference {
public:

	CompositionNodeSlotTemplateGroup(CompositionNodeLogic& owner_, const util::Str8& name_, bool is_input);
	virtual ~CompositionNodeSlotTemplateGroup(){}
	CompositionNodeSlotTemplateGroup(CompositionNodeSlotTemplateGroup&&)= delete;
	CompositionNodeSlotTemplateGroup(const CompositionNodeSlotTemplateGroup&)= delete;

	CompositionNodeSlotTemplateGroup& operator=(const CompositionNodeSlotTemplateGroup&)= delete;
	CompositionNodeSlotTemplateGroup& operator=(CompositionNodeSlotTemplateGroup&&)= delete;
	
	CompositionNodeLogic& getOwner(){ ensure(owner); return *owner; }
	
	void addSlotTemplate(const util::Str8& name, SignalType signaltype);
	SlotTemplate getSlotTemplate(const SlotIdentifier& id);
	bool hasSlotTemplate(const SlotIdentifier& id) const;
	
	/// If true, custom slots can be added and removed in editor
	void setAsVariant(bool b){ variant= b; }
	bool isVariant() const { return variant; }
	
	void setMirroring(bool b){ mirror= b; }
	bool isMirroring() const { return mirror; }
	
	const util::Str8& getName() const { return name; }
	bool isInput() const { return inputType; }
	const util::DynArray<SlotTemplate>& getSlots() const { return slots; }


private:
	CompositionNodeLogic* owner;
	util::Str8 name;
	bool inputType;
	util::DynArray<SlotTemplate> slots;
	bool variant;
	bool mirror; // Add input and output slots as pairs
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::SlotTemplate> {
	static util::Str8 type(){ return "::SlotTemplate"; }
};

template <>
struct TypeStringTraits<nodes::CompositionNodeSlotTemplateGroup> {
	static util::Str8 type(){ return "::CompositionNodeSlotTemplateGroup"; }
};

} // util
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODESLOTTEMPLATEGROUP_HPP