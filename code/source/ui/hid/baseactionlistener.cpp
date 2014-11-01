#include "baseactionlistener.hpp"

namespace clover {
namespace ui { namespace hid {

BaseActionListener::ListenerMap BaseActionListener::outsideActionListeners;

BaseActionListener::BaseActionListener()
	: listening(false){}

void BaseActionListener::onActionListeningStart(const BaseActionListener& listener){
	ensure(!listening);
	outsideActionListeners[listener.getListenerKey()].pushBack(&listener);
	listening= true;
}

void BaseActionListener::onActionListeningStop(const BaseActionListener& listener){
	ensure(listening);
	
	auto& listener_array= outsideActionListeners[listener.getListenerKey()];
	
	for (auto it= listener_array.begin(); it != listener_array.end(); ++it){
		if (*it == &listener){
			listener_array.erase(it);
			listening= false;
			return;
		}
	}
	
	throw global::Exception("ActionListener not found");
}

}} // ui::hid
} // clover