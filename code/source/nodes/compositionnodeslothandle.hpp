#ifndef CLOVER_NODES_COMPOSITIONNODESLOTHANDLE_HPP
#define CLOVER_NODES_COMPOSITIONNODESLOTHANDLE_HPP

#include "build.hpp"
#include "slotidentifier.hpp"

namespace clover {
namespace nodes {

class CompositionNodeLogic;
class CompositionNodeSlot;

/// Slots can be changed (removed & recreated) in script, so can't have direct pointers to them stored
class CompositionNodeSlotHandle {
public:
	CompositionNodeSlotHandle(CompositionNodeSlot& slot);
	
	explicit operator bool() const;
	CompositionNodeSlot* get() const;
	const SlotIdentifier& getIdentifier() const { return identifier; }
	CompositionNodeSlot* operator->() const;
	
	
	bool operator==(const CompositionNodeSlotHandle& other) const { return owner == other.owner && identifier == other.identifier; }
	bool operator!=(const CompositionNodeSlotHandle& other) const { return !(*this == other); }
	
private:
	CompositionNodeLogic* owner;
	SlotIdentifier identifier;
};

} // nodes
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODESLOTHANDLE_HPP