#ifndef CLOVER_NODES_NODEINSTANCE_GUIINPUT_HPP
#define CLOVER_NODES_NODEINSTANCE_GUIINPUT_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"

namespace clover {
namespace nodes {

/// Provides information of gui interactions
class GuiInputNodeInstance : public NodeInstance {
public:
	static CompNode* compNode();

	virtual void create() override;
	virtual void update() override;

private:
	InputSlot<SignalType::String>* channelNameIn;
	OutputSlot<SignalType::Vec2>* cursorPos_world;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::GuiInputNodeInstance> {
	static util::Str8 type(){ return "::GuiInputNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_GUIIINPUT_HPP
