#ifndef CLOVER_UI_HID_CONTROLS_CONTROL_BUTTON_HPP
#define CLOVER_UI_HID_CONTROLS_CONTROL_BUTTON_HPP

#include "../control.hpp"
#include "build.hpp"
#include "util/callbacker.hpp"

namespace clover {
namespace ui { namespace hid {

class ButtonControl : public Control {
public:
	ButtonControl(): prevDown(false), curDown(false){}
	virtual ~ButtonControl(){}
	
	void setValue(bool down){ curDown= down; }
	
	virtual void update() override {
		triggerEvent<nodes::SignalType::Boolean>("value", curDown);
		
		if (!prevDown && curDown){
			triggerEvent("press");
			triggerEvent<nodes::SignalType::Boolean>("valueChanged", true);
			triggerEvent("enabled"); // For context state change control
		}
		else if (prevDown && !curDown){
			triggerEvent("release");
			triggerEvent<nodes::SignalType::Boolean>("valueChanged", false);
			triggerEvent("disabled"); // For context state change control
		}
		
		if (curDown)
			triggerEvent("down");
			
		prevDown= curDown;
	}

private:
	bool prevDown, curDown;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_CONTROLS_CONTROL_BUTTON_HPP