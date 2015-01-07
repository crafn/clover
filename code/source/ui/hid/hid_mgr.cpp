#include "hid_mgr.hpp"
#include "debug/print.hpp"
#include "global/cfg_mgr.hpp"
#include "global/event.hpp"
#include "global/exception.hpp"

namespace clover {
namespace ui { namespace hid {

HidMgr::HidMgr(){
}

void HidMgr::create(){
	devices= global::g_env.cfg->get<decltype(devices)>("hid::devices");
	contexts= global::g_env.cfg->get<decltype(contexts)>("hid::contexts");
	
	removeTagIndirection();
}

void HidMgr::update(){
	for (auto& device : devices){
		device->deviceUpdate();
	}
}

HidMgr::DevicePtr HidMgr::getDevice(const Device::Name& device_name) const {
	for (auto& device : devices){
		if (device->getName() == device_name)
			return device;
	}
	
	throw global::Exception("ui::hid::Device not found: %s", device_name.cStr());
}

HidMgr::DevicePtrs HidMgr::getDevicesByTypeName(const Device::Name& device_type_name, bool only_which_allow_typical_controls) const {
	DevicePtrs ret;
	for (auto& device : devices){
		if (device->getTypeName() != device_type_name)
			continue;
		
		if (only_which_allow_typical_controls && !device->allowsTypicalControls())
			continue;
		
		ret.pushBack(device);
	}
	return (ret);
}

ContextChannel::Names HidMgr::getChannelNames() const {
	ContextChannel::Names ret;
	for (auto& device : devices){
		ret.pushBack(device->getChannelNames());
		ret= util::duplicatesRemoved(ret);
	}
	return (ret);
}

void HidMgr::removeTagIndirection(){
	// Plan: start traversing enableTags of a context, adding previous contexts' tags to the tags of current context
	// Repeat for every context
	std::function<bool(Context&, const Context::Tags&)> add_tags = 
	[&] (Context& cur_ctx, const Context::Tags& enabling_tags) -> bool {
		if (util::duplicates(cur_ctx.getTags(), enabling_tags).size() == enabling_tags.size())
			return false; // This context has no new tags to offer
		
		for (const auto& enabling_tag : enabling_tags){
			cur_ctx.tryAddTag(enabling_tag); // Add tags from "parent" context
		}
		
		for (auto& context : getContexts(cur_ctx.getEnableTags())){
			add_tags(*context, cur_ctx.getTags()); // Be the "parent" context
		}
		
		return true;
	};
	
	for (auto& context : contexts){
		for (auto& enable_context : getContexts(context->getEnableTags())){
			add_tags(*enable_context, context->getTags());
		}
	}
	
	// Remove indirection from enableTags also
	// Plan: copy enableTags from to-be-enabled contexts
	// Repeat for every context
	for (auto& context : contexts){
		Context::Tags new_enable_tags= context->getEnableTags();
		
		for (const auto& old_enable_tag : context->getEnableTags()){
			for (const auto& enable_context : getContexts(context->getEnableTags())){
				new_enable_tags.pushBack(enable_context->getEnableTags());
			}
		}
		
		for (const auto& new_tag : new_enable_tags){
			// There will be some duplicates in new_enable_tags but tryAddEnabledTag filters them away
			context->tryAddEnableTag(new_tag);
		}
	}
#if 0
	for (auto& context : contexts){
		print(debug::Ch::General, debug::Vb::Trivial, "Context");
		for (auto& tag : context->getTags()){
			print(debug::Ch::General, debug::Vb::Trivial, "	   %s", tag.cStr());
		}
		print(debug::Ch::General, debug::Vb::Trivial, "Enabletags");
		for (auto& tag : context->getEnableTags()){
			print(debug::Ch::General, debug::Vb::Trivial, "	   %s", tag.cStr());
		}
	}
#endif
}

util::DynArray<Context::Ptr> HidMgr::getContexts(const Context::Tags& tags){
	util::DynArray<Context::Ptr> ret;
	for (const auto& context : contexts){
		for (const auto& tag : tags){
			if (context->hasTag(tag)){
				ret.pushBack(context);
				break;
			}
		}
	}
	return (ret);
}

}} // ui::hid
} // clover