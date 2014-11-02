#ifndef CLOVER_UI_HID_CONTEXT_HPP
#define CLOVER_UI_HID_CONTEXT_HPP

#include "build.hpp"
#include "global/eventreceiver.hpp"
#include "contextchannel.hpp"
#include "virtualcontrol.hpp"
#include "util/objectnode.hpp"

#include <memory>

namespace clover {
namespace ui { namespace hid {

class BaseActionListener;

/// Context in which VirtualControls are defined.
/// Context sends actions from VirtualControls it owns to ContextChannels
/// It also manages channel enabling/disabling based on current Tags
/// @note	Indirection of tag enabling through multiple contexts is removed from tag functions
///			which means that 'getTags().size() >= "tags" && getEnableTags().size() >= "enableTags"'
class Context : public util::Callbacker<OnActionCb>, public global::EventReceiver {
public:
	using Ptr= std::shared_ptr<Context>;
	using Tag= util::Str8;
	using Tags= util::DynArray<Tag>;
	using ControlPtr= ContextChannel::ControlPtr;
	
	static Ptr create(const Tags& tags, const Tags& enable_tags){
		return Ptr(new Context(tags, enable_tags)); }
		
	void addVirtualControl(VirtualControl::Ptr ptr);
	
	/// @return Tags which can enable this context ("tags" + other contexts' "enableTags" from cfg point of view)
	const Tags& getTags() const { return tags; }
	
	/// @return true if context can be enabled with tag t. Always false if t == ""
	bool hasTag(const Tag& t) const;
	
	/// @return Tags which are enabled at the same time than this context ("enableTags" + other contexts' "tags" from cfg point of view)
	const Tags& getEnableTags() const { return enableTags; }
	
	virtual void onEvent(global::Event& e);
	
	void tryAddTag(const Tag& tag);
	void tryAddEnableTag(const Tag& tag);
	
	/// @return State controls of all channels
	util::DynArray<ControlPtr> getStateControls() const;
	
protected:
	friend class util::ObjectNodeTraits<ui::hid::Context::Ptr>;
	
	const util::DynArray<VirtualControl::Ptr>& getVirtualControls() const { return virtualControls; }
	
private:
	Context(const Tags& tags, const Tags& enable_tags);
	
	void onControlAction(const Action& action, const VirtualControl& virtual_control);
	
	ContextChannel& getChannel(const ContextChannel::Name& name);
	
	Tags tags;
	Tags enableTags;
	util::DynArray<VirtualControl::Ptr> virtualControls;
	
	/// Listens for control actions
	util::CbMultiListener<OnActionCb> actionListener;
	
	/// VirtualControl actions are forwarded to contextChannels
	util::HashMap<ContextChannel::Name, ContextChannel> contextChannels;
};

}} // ui::hid
namespace util {

template <>
struct ObjectNodeTraits<ui::hid::Context::Ptr> {
	typedef ui::hid::Context::Ptr Value;
	static util::ObjectNode serialized(const Value& value);
	static Value deserialized(const util::ObjectNode& ob_node);
};

} // util
} // clover

#endif // CLOVER_UI_HID_CONTEXT_HPP