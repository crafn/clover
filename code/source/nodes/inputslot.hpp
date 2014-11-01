#ifndef CLOVER_NODES_INPUTSLOT_HPP
#define CLOVER_NODES_INPUTSLOT_HPP

#include "build.hpp"
#include "baseinputslot.hpp"
#include "signaltypetraits.hpp"
#include "util/any.hpp"

// .tpp
#include "resources/exception.hpp"

namespace clover {
namespace nodes {

template <SignalType S>
class InputSlot : public BaseInputSlot {
public:
	typedef typename SignalTypeTraits<S>::Value Value;

	InputSlot();
	virtual ~InputSlot();

	/// Throws and resources::ResourceException if value is incorrect type
	virtual void setDefaultValue(const util::Any& value);

	/// Doesn't trigger callbacks
	void set(const Value& v){ value= v; }
	virtual void set(const util::Any& value) override;
	virtual void subSet(SubSignalType sub_from, const util::Any& sub_value, SubSignalType sub_to) override;
	
	/// @return Last received value
	const Value& get() const;
	virtual util::Any getAny() const override { return get(); }
	
	/// OutputSlot calls
	void receive(const Value& value);

private:
	Value value;
};

#include "inputslot.tpp"

} // nodes
namespace util {

template <nodes::SignalType S>
struct TypeStringTraits<nodes::InputSlot<S>> {
	static util::Str8 type(){ return util::Str8("::") + nodes::SignalTypeTraits<S>::enumString() + "InputSlot"; }
};

} // util
} // clover

#endif // CLOVER_NODES_INPUTSLOT_HPP