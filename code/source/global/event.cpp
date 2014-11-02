#include "event.hpp"
#include "eventqueue.hpp"
#include "global/exception.hpp"
#include "eventreceiver.hpp"
#include "event_mgr.hpp"
#include "game/worldentity.hpp"

namespace clover {
namespace global {

int32 Event::EventTypeVariables::getVarCount() const{
	for (int32 i=0; i<maxVariables; i++)
		if (vars[i] == Event::NoneVariable)return i;

	return maxVariables;
}

const Event::EventTypeVariables Event::eventTypeVariables[Event::LastEvent]={
	{NoneVariable},
	#define EVT(a, ...) {{__VA_ARGS__}},
	#define VAR(b)
	#include "events.def"
	#undef VAR
	#undef EVT
};

const util::Str8 Event::eventNames[Event::LastEvent]{
	"NoneEvt",
	#define EVT(a, ...) #a,
	#define VAR(b)
	#include "events.def"
	#undef VAR
	#undef EVT
};

const util::Str8 Event::variableNames[Event::LastVariable]{
	"NoneVar",
	#define EVT(a, ...)
	#define VAR(b) #b,
	#include "events.def"
	#undef VAR
	#undef EVT
};

Event::Event(EType t)
		: type(t)
		, queued(false){

}

Event::~Event(){
	if (!queued)
		queue();
}

Event::EType Event::getType() const {
	return type;
}

util::Str8 Event::getName() const {
	return eventNames[type];
}

bool Event::hasVariable(VariableType s){
	if (vars.count(s))
		return true;

	return false;
}

bool Event::canHaveVariable(VariableType s){
	for (int32 i=0; i<eventTypeVariables[type].getVarCount(); ++i)
		if (eventTypeVariables[type][i] == s) return true;
		
	return false;
}

util::Variant& Event::operator()(VariableType s){
	ensure(canHaveVariable(s));
	return vars[s];
}

void Event::addReceiver(const game::WeHandle& h){
	receivers.add(h);
}

void Event::addReceivers(const game::WESet& h){
	receivers.add(h);
}

int32 Event::getReceiverCount(){
	return receivers.size();
}

void Event::queue(){
	queued= true;
	global::gEventQueue.queue(*this);
}

void Event::send(){
	queued= true;
	
	auto registered_receivers= global::gEventMgr->getRegisteredReceivers(type);
	for (auto &m : registered_receivers){
		m->onEvent(*this);
	}

	game::WESet::Iter it;

	for (it= receivers.begin(); it!=receivers.end(); it++){
		if ((*it)){
			it->get()->onEvent(*this);
			//print(debug::Ch::Event, debug::Vb::Trivial, "Event sent");
		}
	}
}

} // global
} // clover