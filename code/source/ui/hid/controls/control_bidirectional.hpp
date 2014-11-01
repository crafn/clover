#ifndef CLOVER_UI_HID_CONTROLS_CONTROL_BIDIRECTIONAL_HPP
#define CLOVER_UI_HID_CONTROLS_CONTROL_BIDIRECTIONAL_HPP

#include "../control.hpp"
#include "build.hpp"
#include "util/callbacker.hpp"
#include "util/vector.hpp"

namespace clover {
namespace ui { namespace hid {

class BiDirectionalControl : public Control {
public:
	using Value= nodes::SignalTypeTraits<nodes::SignalType::Vec2>::Value;
	
	BiDirectionalControl(){}
	virtual ~BiDirectionalControl(){}
	
	/// Convention is to use normalized values (components between -1 and 1)
	void setValue(const Value& value){
		prevValue= curValue;
		curValue= value;
	}
	
	virtual void update() override {
		auto delta= curValue - prevValue;

		triggerEvent<nodes::SignalType::Vec2>("value", curValue);
		triggerEvent<nodes::SignalType::Vec2>("delta", curValue - prevValue);

		if (prevValue != curValue)
			triggerEvent<nodes::SignalType::Vec2>("changedValue", curValue);
		if (delta != prevDelta)
			triggerEvent<nodes::SignalType::Vec2>("changedDelta", curValue - prevValue);

		prevDelta= curValue - prevValue;
		
		/// @todo Custom step value
		if (std::floor(curValue.y) > std::floor(prevValue.y))
			triggerEvent("stepUp");
		else if (std::floor(curValue.y) < std::floor(prevValue.y))
			triggerEvent("stepDown");
	}
	
private:
	Value curValue, prevValue, prevDelta;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_CONTROLS_CONTROL_BIDIRECTIONAL_HPP
