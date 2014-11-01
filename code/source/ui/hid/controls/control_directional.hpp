#ifndef CLOVER_UI_HID_CONTROLS_CONTROL_DIRECTIONAL_HPP
#define CLOVER_UI_HID_CONTROLS_CONTROL_DIRECTIONAL_HPP

#include "../control.hpp"
#include "build.hpp"
#include "util/callbacker.hpp"
#include "util/vector.hpp"

namespace clover {
namespace ui { namespace hid {

class DirectionalControl : public Control {
public:
	using Value= typename nodes::SignalTypeTraits<nodes::SignalType::Real>::Value;
	
	DirectionalControl()
		: curValue(0.0)
		, prevValue(0.0){
	}
	virtual ~DirectionalControl(){}
	
	void setValue(const Value& value){ curValue= value; }
	
	virtual void update() override {
		triggerEvent<nodes::SignalType::Real>("value", curValue);
		triggerEvent<nodes::SignalType::Real>("delta", curValue - prevValue);
		
		if (prevValue != curValue)
			triggerEvent<nodes::SignalType::Real>("changedValue", curValue);

		prevValue= curValue;
	}
	
private:
	Value curValue, prevValue;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_CONTROLS_CONTROL_DIRECTIONAL_HPP
