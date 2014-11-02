#ifndef CLOVER_NODES_BASEINPUSTSLOT_HPP
#define CLOVER_NODES_BASEINPUSTSLOT_HPP

#include "build.hpp"
#include "baseslot.hpp"

#include <functional>
/// @todo Replace with util::Any
#include <boost/any.hpp>

namespace clover {
namespace nodes {

template <SignalType S>
class InputSlot;

class BaseInputSlot : public BaseSlot {
public:
	typedef std::function<void ()> CallbackType;
	
	BaseInputSlot(SignalType t);
	virtual ~BaseInputSlot();
	
	virtual void setDefaultValue(const boost::any& value) = 0;
	
	/// Doesn't trigger callbacks
	virtual void set(const boost::any& value)= 0;
	virtual void subSet(SubSignalType sub_from, const boost::any& sub_value, SubSignalType sub_to)= 0;
	
	virtual boost::any getAny() const = 0;
	template <SubSignalType S>
	typename SignalTypeTraits<SubSignalTypeTraits<S>::signalType>::Value subGet() const;
	
	void setOnReceiveCallback(const CallbackType& c);
	
	/// Call callback if signal has been received
	void update();
	
	/// Sets receive-callback to be called in update
	void setValueReceived();
	
	template <SignalType S>
	InputSlot<S>& casted(){
		ensure(S == getType());
		return *static_cast<InputSlot<S>*>(this);
	}
	
	/// Sets an external variable to be modified when signal is sent
	void setExtValueReceived(bool* ext_value_received){ extValueReceived= ext_value_received; }
	
private:
	bool valueReceived;
	/// Provides a way to have an external flag to be set when needs update
	/// Many slots can point to the same external flag so slots should modify it only by setting it to true
	bool* extValueReceived= nullptr;
	
	std::function<void ()> onReceiveCallback;
};

template <SubSignalType S>
typename SignalTypeTraits<SubSignalTypeTraits<S>::signalType>::Value BaseInputSlot::subGet() const {
	return RuntimeSignalTypeTraits::extract(getType(), S, getAny()).get<S>();
}

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::BaseInputSlot> {
	static util::Str8 type(){ return "::BaseInputSlot"; } 
};

} // util
} // clover

#endif // CLOVER_NODES_BASEINPUSTSLOT_HPP