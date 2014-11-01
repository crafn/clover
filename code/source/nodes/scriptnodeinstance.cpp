#include "scriptnodeinstance.hpp"
#include "compositionnodelogic.hpp"
#include "nodetype.hpp"

namespace clover {
namespace nodes {

BaseScriptNodeInstance::BaseScriptNodeInstance():
	owner(nullptr){
	
}

void BaseScriptNodeInstance::setOwner(ScriptNodeInstance& owner_){
	owner= &owner_;
}

#define SIGNAL(x, n) \
	InputSlot<SignalType::x>* BaseScriptNodeInstance::add ## x ## InputSlot(const util::Str8& name){ \
		ensure(owner); \
		return owner->addInputSlot<SignalType::x>(name); \
	} \
	OutputSlot<SignalType::x>* BaseScriptNodeInstance::add ## x ## OutputSlot(const util::Str8& name){ \
		ensure(owner); \
		return owner->addOutputSlot<SignalType::x>(name); \
	} \
	InputSlot<SignalType::x>* BaseScriptNodeInstance::add ## x ## InputSlot(const CompositionNodeSlot& slot){ \
		ensure(owner); \
		return owner->addInputSlot<SignalType::x>(slot); \
	} \
	OutputSlot<SignalType::x>* BaseScriptNodeInstance::add ## x ## OutputSlot(const CompositionNodeSlot& slot){ \
		ensure(owner); \
		return owner->addOutputSlot<SignalType::x>(slot); \
	}
#include "signaltypes.def"
#undef SIGNAL

void BaseScriptNodeInstance::setUpdateNeeded(bool b){
	owner->setUpdateNeeded(b);
}

bool BaseScriptNodeInstance::isUpdateNeeded() const {
	return owner->isUpdateNeeded();
}

ScriptNodeInstance::ScriptNodeInstance(const NodeType& type, const script::ObjectType& obj_type, script::Context& context_)
		: context(context_)
		, impl(nullptr){
	
	try {
		object= std::move(context.instantiateObject(obj_type));
		
		updateFunc= obj_type.getMethod<void ()>("update");
		
		auto get_impl= obj_type.getMethod<BaseScriptNodeInstance& ()>("getImpl");
		impl= &context.execute(object, get_impl());
		ensure(impl);
		
		impl->setOwner(*this);
	}
	catch (const resources::ResourceException& e){
		throw resources::ResourceException("ScriptNodeInstance constructing failed for %s", type.getName().cStr());
	}
}

ScriptNodeInstance::~ScriptNodeInstance(){
}

void ScriptNodeInstance::create(){
	auto add_template_slot= object.getType().getMethod<void (CompositionNodeSlot*)>("addTemplateSlot");
	for (CompositionNodeSlot* slot : compositionNodeLogic->getTemplateGroupSlots()){
		context.execute(object, add_template_slot(slot));
	}
	
	auto create= object.getType().getMethod<void ()>("create");
	context.execute(object, create());
	
	for (auto& m : getInputSlots()){
		auto on_receive= object.getType().getMethod<void (const BaseInputSlot*)>("onReceive");
		m.slot->setOnReceiveCallback([=, &m] () {
			context.execute(object, on_receive(m.slot.get()));
		});
	}
}

void ScriptNodeInstance::update(){
	context.execute(object, updateFunc());
}

} // nodes
} // clover