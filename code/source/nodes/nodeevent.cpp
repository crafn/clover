#include "nodeevent.hpp"
#include "game/worldentity_handle.hpp"
#include "game/worldentity_set.hpp"
#include "global/env.hpp"
#include "nodeevent_mgr.hpp"
#include "nodeeventreceiverproxy_we.hpp"
#include "resources/cache.hpp"

namespace clover {
namespace nodes {

NodeEvent::NodeEvent()
		: type(nullptr)
		, queued(false){
}

NodeEvent::NodeEvent(const util::Str8& type_name)
		: NodeEvent(global::g_env.resCache->getResource<NodeEventType>(type_name)){
}

NodeEvent::NodeEvent(const NodeEventType& t)
		: type(&t)
		, queued(false){
}

NodeEvent::NodeEvent(const NodeEvent& other){
	*this= other;
}

NodeEvent& NodeEvent::operator=(const NodeEvent& other){
	type= other.type;
	arguments= other.arguments;
	queued= other.queued;
	
	for (auto& m : other.receivers){
		receivers.pushBack(std::unique_ptr<NodeEventReceiverProxy>(m->clone()));
	}
	
	return *this;
}

NodeEvent::~NodeEvent(){
}


const boost::any& NodeEvent::get(const util::Str8& name) const {
	auto it= arguments.find(name);
	ensure(it != arguments.end());
	return it->second;
}

void NodeEvent::set(const util::Str8& name, const boost::any& value){
	arguments[name]= value;
}

void NodeEvent::addReceiver(const game::WorldEntity& recv){
	receivers.pushBack(std::move(
		std::unique_ptr<NodeEventReceiverProxy>(
			new WeNodeEventReceiverProxy(game::WeHandle{&recv}))
	));
}

void NodeEvent::addReceiver(const game::WESet& recv){
	for (auto& m : recv){
		addReceiver(m.ref());
	}
}

void NodeEvent::send(){
	for (auto& m : receivers){
		m->onEvent(*this);
	}
}

void NodeEvent::queue(){
	ensure(!queued);
	gNodeEventMgr->queue(*this);
	queued= true;
}

const util::Str8& NodeEvent::getName() const {
	if (!type) return getNullName();
	return type->getName();
}

SizeType NodeEvent::getArgCount() const {
	return arguments.size();
}

} // nodes
} // clover
