#ifndef CLOVER_NODES_COMPOSITIONNODESLOTTEMPLATEGROUPHANDLE_HPP
#define CLOVER_NODES_COMPOSITIONNODESLOTTEMPLATEGROUPHANDLE_HPP

#include "build.hpp"
#include "util/string.hpp"

namespace clover {
namespace nodes {

class CompositionNodeLogic;
class CompositionNodeSlotTemplateGroup;

class CompositionNodeSlotTemplateGroupHandle {
public:
	CompositionNodeSlotTemplateGroupHandle(CompositionNodeSlotTemplateGroup* group= nullptr);
	
	explicit operator bool() const;
	CompositionNodeSlotTemplateGroup* get() const;
	const util::Str8& getName() const { return name; }
	CompositionNodeSlotTemplateGroup* operator->() const;
	
	
	bool operator==(const CompositionNodeSlotTemplateGroupHandle& other) const { return owner == other.owner && name == other.name; }
	bool operator!=(const CompositionNodeSlotTemplateGroupHandle& other) const { return !(*this == other); }
	
private:
	CompositionNodeLogic* owner;
	util::Str8 name; /// Group name
};

} // nodes
} // clover

#endif // CLOVER_NODES_COMPOSITIONNODESLOTTEMPLATEGROUPHANDLE_HPP