#include "contextchannel.hpp"
#include "baseactionlistener.hpp"
#include "context.hpp"
#include "controls/control_button.hpp"

namespace clover {
namespace ui { namespace hid {

ContextChannel::ContextChannel(const Name& name_, const Context& owner_)
	: owner(&owner_)
	, enabled(false)
	, name(name_)
	, stateControl(new ButtonControl()){
	stateControl->addChannel(name);
	stateControl->setName("state");
}

void ContextChannel::setEnabled(bool e){
	ButtonControl* ctrl= dynamic_cast<ButtonControl*>(stateControl.get());
	ensure(ctrl);
	if (!e){
		ctrl->setValue(e); // Must process before so that context delivers disable action
		ctrl->update();
	}
	
	enabled= e;
	
	if (e){
		ctrl->setValue(e); // Must process after so that context delivers enable action
		ctrl->update();
	}
}

void ContextChannel::onAction(const Action& action){
	if (!enabled)
		return;
	
	/// @todo detect sequences
	
	for (const auto& tag : owner->getTags()){
		
		/// Trigger corresponding ActionListener callbacks
		const auto& outside_listeners=	BaseActionListener::getListeners(
											BaseActionListener::ListenersKey(name, tag, action.getName())
										);
										
		for (const auto& outside_listener : outside_listeners){
			outside_listener->onAction(action);
		}
		
	}
}

}} // ui::hid
} // clover