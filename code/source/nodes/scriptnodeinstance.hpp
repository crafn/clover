#ifndef CLOVER_NODES_NODEINSTANCESCRIPTLOGIC_HPP
#define CLOVER_NODES_NODEINSTANCESCRIPTLOGIC_HPP

#include "build.hpp"
#include "nodeinstance.hpp"
#include "script/context.hpp"
#include "script/objecttype.hpp"

namespace clover {
namespace nodes {

class NodeType;
class ScriptNodeInstance;

/// Connection to script
class BaseScriptNodeInstance {
public:
	
	BaseScriptNodeInstance();
	void setOwner(ScriptNodeInstance& owner);
	
	/// Called from script
	
	#define SIGNAL(x, n) \
		InputSlot<SignalType::x>* add ## x ## InputSlot(const util::Str8& name); \
		OutputSlot<SignalType::x>* add ## x ## OutputSlot(const util::Str8& name); \
		InputSlot<SignalType::x>* add ## x ## InputSlot(const CompositionNodeSlot& slot); \
		OutputSlot<SignalType::x>* add ## x ## OutputSlot(const CompositionNodeSlot& slot);
	#include "signaltypes.def"
	#undef SIGNAL
	
	void setUpdateNeeded(bool b);
	bool isUpdateNeeded() const;
	
private:

	ScriptNodeInstance* owner;
};

class ScriptNodeInstance : public NodeInstance {
public:
	ScriptNodeInstance(const NodeType& type, const script::ObjectType& obj_type, script::Context& context);
	virtual ~ScriptNodeInstance();
	
	virtual void create();
	virtual void update();
	
	void setUpdateNeeded(bool b){ NodeInstance::setUpdateNeeded(b); }
	
private:
	// Ugly hack to have const getInputSlot and getOutputSlot
	mutable script::Context context;
	mutable script::Object object;
	
	script::Function<void ()> updateFunc;
	
	BaseScriptNodeInstance* impl;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::BaseScriptNodeInstance> {
	static util::Str8 type(){ return "::BaseScriptNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCESCRIPTLOGIC_HPP