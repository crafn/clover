#ifndef CLOVER_UI_HID_ACTION_HPP
#define CLOVER_UI_HID_ACTION_HPP

#include "build.hpp"
#include "nodes/signaltypetraits.hpp"
#include "util/callbacker.hpp"
#include "util/string.hpp"

namespace clover {
namespace ui { namespace hid {

class Action {
public:
	using Name= util::Str8;
	
	Action(const Name& name_, const nodes::SignalValue& v): name(name_), value(v){}
	
	const util::Str8& getName() const { return name; }
	
	template <nodes::SignalType S>
	void setValue(const typename nodes::SignalTypeTraits<S>::Value& v){ value.set(S, v); }
	
	const nodes::SignalValue& getValue() const { return value; }
	
private:
	util::Str8 name;
	nodes::SignalValue value;
};

}} // ui::hid
} // clover

#endif // CLOVER_UI_HID_ACTION_HPP