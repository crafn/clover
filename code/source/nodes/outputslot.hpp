#ifndef CLOVER_NODES_OUTPUTSLOT_HPP
#define CLOVER_NODES_OUTPUTSLOT_HPP

#include "build.hpp"
#include "baseoutputslot.hpp"
#include "signaltypetraits.hpp"
#include "inputslot.hpp"
#include "util/any.hpp"
#include "util/ensure.hpp"
// .tpp
#include "debug/debugprint.hpp"

namespace clover {
namespace nodes {

template <SignalType S>
class OutputSlot : public BaseOutputSlot {
public:
	typedef typename SignalTypeTraits<S>::Value Value;

	OutputSlot();
	virtual ~OutputSlot();

	void send(const Value& value) const;
	virtual void send(const util::Any& value) const;
	
	template <SignalType T= S>
	typename std::enable_if<T == SignalType::Trigger>::type
	send() const { send(TriggerValue()); }
	
private:
	template <SignalType T>
	void sendImpl(BaseInputSlot& slot, const typename SignalTypeTraits<T>::Value& value) const;

	template <SignalType T>
	const typename SignalTypeTraits<T>::Value& getInputValue(BaseInputSlot& slot) const;
};

#include "outputslot.tpp"

} // nodes
namespace util {

template <nodes::SignalType S>
struct TypeStringTraits<nodes::OutputSlot<S>> {
	static util::Str8 type(){ return util::Str8("::") + nodes::SignalTypeTraits<S>::enumString() + "OutputSlot"; }
};

} // util
} // clover

#endif // CLOVER_NODES_OUTPUTSLOT_HPP