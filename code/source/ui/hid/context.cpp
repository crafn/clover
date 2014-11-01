#include "context.hpp"
#include "actionlistener.hpp"
#include "hid_mgr.hpp"
#include "hardware/device.hpp"
#include "hardware/hid_devicefactory.hpp"
#include "virtualcontrol.hpp"

namespace clover {
namespace ui { namespace hid {

Context::Context(const Tags& tags_, const Tags& enable_tags_)
	: tags(tags_)
	, enableTags(enable_tags_){
	
	for (const auto& m : tags){
		if (m.empty())
			throw global::Exception("Empty tag not allowed"); // Empty tags not allowed, so hasTag("") == false
	}
		
	for (const auto& m : enableTags){
		if (m.empty())
			throw global::Exception("Empty enableTag not allowed"); // Empty tags not allowed, so hasTag("") == false
	}
	
	// Create channels before virtualControls are added, because virtualControls might need state controls from channels
	for (const auto& ch : hardware::gDevice->getHidMgr().getChannelNames()){
		// get creates missing channel
		getChannel(ch);
	}
	
	listenForEvent(global::Event::OnHidTagChange);
}

void Context::addVirtualControl(VirtualControl::Ptr ptr){
	virtualControls.pushBack(std::move(ptr));

	actionListener.listen(*virtualControls.back(), std::bind(&Context::onControlAction, this, std::placeholders::_1, std::placeholders::_2));
}

bool Context::hasTag(const Tag& t) const {
	for (const auto& tag : tags){
		if (tag == t)
			return true;
	}
	return false;
}

void Context::onEvent(global::Event& e){
	switch (e.getType()){
		
		case global::Event::OnHidTagChange: {
			ContextChannel::Name channel_name= e(global::Event::ContextChannelName).getStr();
			// tag is empty if last tag was removed from tag list
			Tag tag= e(global::Event::Tag).getStr();
			//print(debug::Ch::Ui, debug::Vb::Trivial, "Hid tag changed for channel: %s, %s, %i", input_channel_name.cStr(), tag.cStr(), hasTag(tag));
			
			getChannel(channel_name).setEnabled(hasTag(tag));
		}
		break;
		
		default: release_ensure(0);
	}
}

void Context::tryAddTag(const Tag& tag){
	if (tags.find(tag) == tags.end())
		tags.pushBack(tag);
}

void Context::tryAddEnableTag(const Tag& tag){
	if (enableTags.find(tag) == enableTags.end())
		enableTags.pushBack(tag);
}

util::DynArray<Context::ControlPtr> Context::getStateControls() const {
	util::DynArray<ControlPtr> ret;
	for (const auto& pair : contextChannels){
		ret.pushBack(pair.second.getStateControl());
	}
	return ret;
}

void Context::onControlAction(const Action& action, const VirtualControl& virtual_control){
	for (const auto& control : virtual_control.getControls()){
		for (const auto& channel_name : control.lock()->getChannelNames()){
			// Send actions to channels
			getChannel(channel_name).onAction(action);
			
			/// @todo Forward filtered_action to contexts who explicitly want actions from this context
		}
	}
}

ContextChannel& Context::getChannel(const ContextChannel::Name& channel_name){
	auto it= contextChannels.find(channel_name);
	if (it == contextChannels.end()){
		ContextChannel new_channel(channel_name, *this);
		contextChannels.insert(std::make_pair(channel_name, new_channel));
		
		return getChannel(channel_name);
	}
	else {
		return it->second;
	}
}
}} // ui::hid
namespace util {

using namespace ui::hid;

util::ObjectNode ObjectNodeTraits<ui::hid::Context::Ptr>::serialized(const Value& context){
	util::ObjectNode ob(util::ObjectNode::Value::Object);
	
	ob["tags"].setValue(context->getTags());
	ob["enableTags"].setValue(context->getEnableTags());
	ob["virtualControls"]= util::ObjectNode(util::ObjectNode::Value::Array);
	
	for (SizeType i= 0; i < context->getVirtualControls().size(); ++i){
		util::ObjectNode virtual_control_ob;
		virtual_control_ob.setValue(context->getVirtualControls()[i]);
		
		ob["virtualControls"].append(virtual_control_ob);
	}
	
	return (ob);
}

auto ObjectNodeTraits<ui::hid::Context::Ptr>::deserialized(const util::ObjectNode& ob) -> Value {
	HidMgr& hid_mgr= hardware::gDevice->getHidMgr();
	
	util::ObjectNode tags_ob= ob.get("tags");
	util::ObjectNode enable_tags_ob= ob.get("enableTags", util::ObjectNode::Value::Null);
	
	// Create context

	Context::Tags tags, enable_tags;
	tags= tags_ob.getValue<Context::Tags>();
	
	if (!enable_tags_ob.isNull())
		enable_tags= enable_tags_ob.getValue<Context::Tags>();
	
	Context::Ptr context= Context::create(tags, enable_tags);
	
	
	// Deserialize VirtualControls
	
	util::ObjectNode virtual_controls_ob= ob.get("virtualControls", util::ObjectNode::Value::Array);
	/// @todo const util::ObjectNode sequences_ob= ob.get("sequences", util::ObjectNode::Value::Array);
	
	util::DynArray<VirtualControl::Ptr> virtual_controls;
	for (SizeType i= 0; i < virtual_controls_ob.size(); ++i){
		const util::ObjectNode virtual_control_ob= virtual_controls_ob.get(i);
		
		try {
			virtual_controls.pushBack(ObjectNodeTraits<VirtualControl::Ptr>::deserialized(virtual_control_ob, *context));
		}
		catch (const global::Exception& e){
			print(debug::Ch::Device, debug::Vb::Moderate, "Error creating ui::hid::VirtualControl: %s", e.what());
		}
	}
	
	for (auto& virtual_control : virtual_controls)
		context->addVirtualControl(virtual_control);
	
	return (context);
}

} // util
} // clover