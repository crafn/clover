#ifndef CLOVER_UI_HID_CONTROLS_CONTROL_TEXT_HPP
#define CLOVER_UI_HID_CONTROLS_CONTROL_TEXT_HPP

#include "../control.hpp"
#include "build.hpp"
#include "util/string.hpp"

namespace clover {
namespace ui { namespace hid {

/// Manages written text
class TextControl : public Control {
public:
	virtual ~TextControl(){}
	
	void append(uint32 unicode){
		newText += unicode;
	}
	
	virtual void update() override {
		triggerEvent<nodes::SignalType::String>("delta", newText);
		if (!newText.empty())
			triggerEvent<nodes::SignalType::String>("new", newText);
		
		newText.clear();
	}
	
private:
	util::Str8 newText;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_CONTROLS_CONTROL_TEXT_HPP