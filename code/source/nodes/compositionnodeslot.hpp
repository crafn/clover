#ifndef CLOVER_NODES_COMPOSITIONNODESLOT_HPP
#define CLOVER_NODES_COMPOSITIONNODESLOT_HPP

#include "build.hpp"
#include "slotidentifier.hpp"
#include "script/reference.hpp"
#include "signaltypetraits.hpp"
#include "compositionnodeslothandle.hpp"
#include "compositionnodeslottemplategrouphandle.hpp"
#include "util/dyn_array.hpp"
#include "util/string.hpp"

#include <initializer_list>
#include <memory>
#include <boost/any.hpp>

namespace clover {
namespace nodes {

class CompositionNodeLogic;
class CompositionNodeSlotTemplateGroup;

class CompositionNodeSlot : public script::NoCountReference {
public:

	struct AttachedSlotInfo {
		CompositionNodeSlotHandle slot;
		SubSignalType mySub;
		SubSignalType slotSub;

		bool operator==(const AttachedSlotInfo& other);
	};

	CompositionNodeSlot(const SlotIdentifier& identifier);
	CompositionNodeSlot& operator=(const CompositionNodeSlot&) = delete;
	CompositionNodeSlot& operator=(CompositionNodeSlot&& other);
	virtual ~CompositionNodeSlot();

	void setOwner(CompositionNodeLogic* owner);
	const CompositionNodeLogic& getOwner() const;
	CompositionNodeLogic& getOwner();

	const util::Str8& getName() const { return identifier.name; }
	SignalType getSignalType() const { return identifier.signalType; }
	const SlotIdentifier& getIdentifier() const { return identifier; }

	bool isInput() const { return identifier.input; }

	void attach(CompositionNodeSlot& other, SubSignalType mysub= SubSignalType::None, SubSignalType othersub= SubSignalType::None);
	void detach(CompositionNodeSlot& other, SubSignalType mysub= SubSignalType::None, SubSignalType othersub= SubSignalType::None);
	void detach();

	bool isAttached() const;
	bool isAttached(SubSignalType to) const;

	/// Only for input-slot
	const util::DynArray<AttachedSlotInfo>& getAttachedSlotInfos() const;

	util::DynArray<CompositionNodeSlot*> getAttachedSlots() const;

	bool isCompatible(SubSignalType from_type, SubSignalType to_type, CompositionNodeSlot& other) const;
	bool hasSubSignalType(SubSignalType type) const;

	void setInitValue(const boost::any& v);

	void setDefaultValue(const boost::any& v);
	void unsetDefaultValue();

	/// If default value is unset, returns the initialization value of the signal type
	const boost::any& getDefaultValue() const;

	template <SignalType S>
	typename SignalTypeTraits<S>::Value getDefaultValue() const {
		return boost::any_cast<typename SignalTypeTraits<S>::Value>(getDefaultValue());
	}

	bool isDefaultValueSet() const { return !defaultValue.empty(); }

	void setTemplateGroup(CompositionNodeSlotTemplateGroup* group){ templateGroup= CompositionNodeSlotTemplateGroupHandle(group); }
	const CompositionNodeSlotTemplateGroup* getTemplateGroup() const { return templateGroup.get(); }

	/// Should be same as getIdentifier().groupName, though groupName can exist when this doesn't
	util::Str8 getTemplateGroupName() const;

private:

	void attachImpl(SubSignalType from, SubSignalType to, CompositionNodeSlot& other);

	CompositionNodeLogic* owner;

	SlotIdentifier identifier;

	util::DynArray<AttachedSlotInfo> attachedSlotInfos;

	/// Value set in script
	boost::any initValue;

	// Value set in editor
	boost::any defaultValue;

	CompositionNodeSlotTemplateGroupHandle templateGroup;

};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::CompositionNodeSlot> {
	static util::Str8 type(){ return "::CompositionNodeSlot"; }
};

} // util
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODESLOT_HPP