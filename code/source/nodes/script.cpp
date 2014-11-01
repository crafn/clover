#include "compositionnodelogic.hpp"
#include "game/worldentity.hpp"
#include "script.hpp"
#include "script/script_mgr.hpp"
#include "scriptnodeinstance.hpp"
#include "signaltypetraits.hpp"
#include "slotidentifier.hpp"
#include "util/unique_ptr.hpp"

namespace clover {
namespace nodes {

/// Registering all node stuff to script
/// Could be done elsewhere

template <typename T>
void registerAddCompositionInputSlotToScript(){
	typedef BaseCompositionNodeScriptLogic This;
	script::gScriptMgr->registerMethod(	static_cast<CompositionNodeSlot& (This::*)(const util::Str8&, const SignalType&, const T&)>(&This::addInputSlot),
										"addInputSlot");
	script::gScriptMgr->registerMethod(	static_cast<CompositionNodeSlot& (This::*)(const util::Str8&, const util::Str8&, const SignalType&, const T&)>(&This::addInputSlot),
										"addInputSlot");
}

template <SignalType S>
void registerGetDefaultValueToScript(){
	typedef CompositionNodeSlot This;
	script::gScriptMgr->registerMethod(	&This::getDefaultValue<S>,
										"getDefault" + SignalTypeTraits<S>::enumString() + "Value");
}

void registerAddCompositionOutputSlotToScript(){
	typedef BaseCompositionNodeScriptLogic This;
	script::gScriptMgr->registerMethod(	static_cast<CompositionNodeSlot& (This::*)(const util::Str8&, const SignalType&)>(&This::addOutputSlot),
										"addOutputSlot");
	script::gScriptMgr->registerMethod(	static_cast<CompositionNodeSlot& (This::*)(const util::Str8&, const util::Str8&, const SignalType&)>(&This::addOutputSlot),
										"addOutputSlot");
}

const util::DynArray<SignalArgument>& getEventArguments(const util::Str8& event_name){
	return resources::gCache->getResource<NodeEventType>(event_name).getArguments();
}

void constructSignalArgument(const util::Str8& name, SignalType s, void* memory){
	new (memory) SignalArgument{name, s};
}

void constructSlotIdentifier(const util::Str8& name, const util::Str8& groupname, SignalType s, bool input, void* memory){
	new (memory) SlotIdentifier{name, groupname, s, input};
}

void registerSignalTypeEnum(script::ScriptMgr& script){
	script.registerEnumType<nodes::SignalType>();
	#define SIGNAL(x, n) script.registerEnumValue<SignalType>(SignalTypeTraits<SignalType::x>::enumString(), n);
	#include "signaltypes.def"
	#undef SIGNAL
}

void registerSignalValueTypes(script::ScriptMgr& script){
	typedef util::Str8 EventTypeValue;
	script.registerObjectType<EventTypeValue>();

	typedef nodes::TriggerValue TriggerValue;
	script.registerObjectType<TriggerValue>();
}

void registerCompositionSlots(script::ScriptMgr& script){

	script.registerObjectType<SlotIdentifier>();
	script.registerConstructor<SlotIdentifier, const util::Str8&, const util::Str8&, SignalType, bool>();
	script.registerMethod(&SlotIdentifier::getString, "getString");
	script.registerMember(&SlotIdentifier::name, "name");
	script.registerMember(&SlotIdentifier::signalType, "signalType");

	typedef nodes::CompositionNodeSlotTemplateGroup TemplateGroup;

	script.registerObjectType<TemplateGroup>();
	script.registerMethod(&TemplateGroup::addSlotTemplate, "addSlotTemplate");
	script.registerMethod(&TemplateGroup::getName, "getName");
	script.registerMethod(&TemplateGroup::setAsVariant, "setAsVariant");
	script.registerMethod(&TemplateGroup::setMirroring, "setMirroring");

	typedef nodes::CompositionNodeSlot CompSlot;
	script.registerObjectType<CompSlot>();
	script.registerMethod(
		static_cast<bool (CompSlot::*)() const>(
			&CompSlot::isAttached),
		"isAttached");
	script.registerMethod(&CompSlot::getName, "getName");
	script.registerMethod(&CompSlot::getSignalType, "getSignalType");
	script.registerMethod(&CompSlot::getTemplateGroup, "getTemplateGroup");
	script.registerMethod(&CompSlot::getIdentifier, "getIdentifier");
}

void registerCompositionLogic(script::ScriptMgr& script){

	typedef BaseCompositionNodeScriptLogic Logic;
	script::gScriptMgr->registerObjectType<Logic>();

	/// @todo Prevent registering input slot with same value type multiple times
	#define SIGNAL(x, n) \
		registerAddCompositionInputSlotToScript<SignalTypeTraits<SignalType::x>::Value>(); \
		registerGetDefaultValueToScript<SignalType::x>();
	#include "signaltypes.def"
	#undef SIGNAL

	registerAddCompositionOutputSlotToScript();

	script.registerMethod(static_cast<CompositionNodeSlot& (Logic::*)(const util::Str8&, const SignalType&)>(&Logic::addInputSlot), "addInputSlot");
	script.registerMethod(static_cast<CompositionNodeSlot& (Logic::*)(const util::Str8&, const util::Str8&, const SignalType&)>(&Logic::addInputSlot), "addInputSlot");

	script.registerMethod(static_cast<CompositionNodeSlot& (Logic::*)(const SlotIdentifier&)>(&Logic::addSlot), "addSlot");
	script.registerMethod(static_cast<void (Logic::*)(const CompositionNodeSlot&)>(&Logic::removeSlot), "removeSlot");

	script.registerMethod(&Logic::getSlot, "getSlot");
	script.registerMethod(&Logic::hasSlot, "hasSlot");

	script.registerMethod(&Logic::addInputSlotTemplateGroup, "addInputSlotTemplateGroup");
	script.registerMethod(&Logic::addOutputSlotTemplateGroup, "addOutputSlotTemplateGroup");

	script.registerMethod(&Logic::setAsUpdateRouteStart, "setAsUpdateRouteStart");
	script.registerMethod(&Logic::setBatched, "setBatched");
	script.registerMethod(&Logic::setBatchPriority, "setBatchPriority");

	script.registerObjectType<resources::Resource*>();
	/// @todo Register type for resource change listening
	script.registerMethod(&Logic::addResourceChangeListener<nodes::NodeEventType>, "addNodeEventTypeChangeListener");
	script.registerMethod(&Logic::clearResourceChangeListeners, "clearResourceChangeListeners");
}

void registerInstance(script::ScriptMgr& script){
	typedef BaseScriptNodeInstance Inst;
	script.registerObjectType<Inst>();
	script.registerMethod(&Inst::isUpdateNeeded, "isUpdateNeeded");
	script.registerMethod(&Inst::setUpdateNeeded, "setUpdateNeeded");
	#define SIGNAL(x, n) \
		script.registerMethod( \
			static_cast<InputSlot<SignalType::x>* (Inst::*)(const util::Str8&)>(&Inst::add ## x ## InputSlot), \
			util::Str8("add" + SignalTypeTraits<SignalType::x>::enumString() + "InputSlot")); \
		script.registerMethod( \
			static_cast<OutputSlot<SignalType::x>* (Inst::*)(const util::Str8&)>(&Inst::add ## x ## OutputSlot), \
			util::Str8("add" + SignalTypeTraits<SignalType::x>::enumString() + "OutputSlot")); \
		script.registerMethod( \
			static_cast<InputSlot<SignalType::x>* (Inst::*)(const CompositionNodeSlot&)>(&Inst::add ## x ## InputSlot), \
			util::Str8("add" + SignalTypeTraits<SignalType::x>::enumString() + "InputSlot")); \
		script.registerMethod( \
			static_cast<OutputSlot<SignalType::x>* (Inst::*)(const CompositionNodeSlot&)>(&Inst::add ## x ## OutputSlot), \
			util::Str8("add" + SignalTypeTraits<SignalType::x>::enumString() + "OutputSlot"));
	#include "signaltypes.def"
	#undef SIGNAL
}

void registerNodeEvent(script::ScriptMgr& script){
	script.registerObjectType<SignalArgument>();
	script.registerConstructor<SignalArgument>(constructSignalArgument);
	script.registerMember(&SignalArgument::name, "name");
	script.registerMember(&SignalArgument::signalType, "signalType");

	typedef nodes::NodeEvent EventValue;
	script.registerObjectType<EventValue>();
	script.registerConstructor<EventValue, const util::Str8&>();
	script.registerMethod<void (EventValue::*)(const game::WorldEntity&)>
		(&EventValue::addReceiver, "addReceiver");
	script.registerMethod(&EventValue::send, "send");
	script.registerMethod(&EventValue::queue, "queue");

#define SIGNAL(x, n) \
	script.registerMethod(&nodes::NodeEvent::set<SignalType::x>, "set"); \
	script.registerMethod(&nodes::NodeEvent::get<SignalType::x>, "get" + \
			SignalTypeTraits<SignalType::x>::enumString());
#include "signaltypes.def"
#undef SIGNAL

	typedef util::DynArray<EventValue> EventArrayValue;
	script.registerObjectType<EventArrayValue>();

	script.registerGlobalFunction(getEventArguments, "getEventArguments");
}

void registerToScript(){

	script::ScriptMgr& script= *script::gScriptMgr;

	registerSignalTypeEnum(script);
	registerSignalValueTypes(script);
	registerNodeEvent(script);
	registerCompositionSlots(script);
	registerCompositionLogic(script);
	registerInstanceSlots_Part1(script);
	registerInstanceSlots_Part2(script);
	registerInstance(script);
	
}

} // nodes
} // clover
