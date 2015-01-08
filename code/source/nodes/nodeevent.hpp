#ifndef CLOVER_NODES_NODEEVENT_HPP
#define CLOVER_NODES_NODEEVENT_HPP

#include "build.hpp"
#include "nodeeventreceiverproxy.hpp"
#include "nodeeventtype.hpp"
#include "signaltypetraits.hpp"

namespace clover {
namespace game {

class WeHandle;
class WESet;

} // game
namespace nodes {

class NodeEvent {
public:
	NodeEvent(); /// Invalid state
	NodeEvent(const util::Str8& type_name);
	NodeEvent(const NodeEventType& type);
	
	NodeEvent(const NodeEvent& other);
	NodeEvent(NodeEvent&&)= default;
	
	NodeEvent& operator=(const NodeEvent& other);
	NodeEvent& operator=(NodeEvent&&)= default;
	
	virtual ~NodeEvent();
	
	const util::Any& get(const util::Str8& name) const;
	void set(const util::Str8& name, const util::Any& value);

	template <SignalType S>
	const typename SignalTypeTraits<S>::Value& get(const util::Str8& name) const {
		return *util::anyCast<typename SignalTypeTraits<S>::Value>(&get(name));
	}
	
	template <SignalType S>
	void set(const util::Str8& name, const typename SignalTypeTraits<S>::Value& value){
		set(name, util::Any(value));
	}

	void addReceiver(const game::WorldEntity& recv);
	void addReceiver(const game::WESet& recv);
	
	/// These will std::move this
	void send();
	void queue(); /// Delayed send
	
	
	template <typename Archive>
	void serialize(Archive& ar, const uint32 ver){
		// Serialization only used with scripts, don't serialize because no default value
	}
	
	const util::Str8& getName() const;
	SizeType getArgCount() const;
	
private:

	static const util::Str8& getNullName(){ static util::Str8 ret("Null"); return ret; }

	const NodeEventType* type;
	util::HashMap<util::Str8, util::Any> arguments;
	
	bool queued;
	util::DynArray<util::UniquePtr<NodeEventReceiverProxy>> receivers;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::NodeEvent> {
	static util::Str8 type(){ return "::NodeEvent"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEEVENT_HPP
