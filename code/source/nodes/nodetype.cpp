#include "nodetype.hpp"
#include "compositionnodelogic.hpp"
#include "nodefactory.hpp"
#include "nodeinstance.hpp"
#include "scriptnodeinstance.hpp"
#include "resources/cache.hpp"
#include "script/module.hpp"
#include "signaltypetraits.hpp"
#include "slotidentifier.hpp"

namespace clover {
namespace nodes {

NodeType::NodeType()
		: scriptModule(nullptr)
		, INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(scriptModuleAttribute, "scriptModule", "")
		, INIT_RESOURCE_ATTRIBUTE(compositionLogicClassAttribute, "compositionLogicScriptClass", "")
		, INIT_RESOURCE_ATTRIBUTE(nativeInstanceClassAttribute, "nativeInstanceClass", "")
{
	scriptModuleAttribute.setOnChangeCallback([&] {tryStartReloading(); });
	compositionLogicClassAttribute.setOnChangeCallback([&] {tryStartReloading(); });
	nativeInstanceClassAttribute.setOnChangeCallback([&] {tryStartReloading(); });
}

NodeType::~NodeType(){

}

void NodeType::resourceUpdate(bool load, bool force){
	if (load || getResourceState() == State::Uninit){
		try {
			const script::Module& module= resources::gCache->getResource<script::Module>(scriptModuleAttribute.get());
			
			moduleChangeListener.clear();
			moduleChangeListener.listen(module, [&] () {
				tryStartReloading();
			});
			
			if (module.getResourceState() != resources::Resource::State::Loaded)
				throw resources::ResourceException("Error loading NodeType %s, script module %s in error state", 
					getName().cStr(), module.getName().cStr());
			
			compositionLogicObjectType= module.getObjectType(compositionLogicClassAttribute.get());

			setResourceState(State::Loaded);
		}
		catch (resources::ResourceException){
			createErrorResource();
		}
	}
	else {
		moduleChangeListener.clear();
		setResourceState(State::Unloaded);
	}
}

void NodeType::createErrorResource(){
	setResourceState(State::Error);
}

CompositionNodeLogic* NodeType::createCompositionLogic(script::Context& context) const {
	return new CompositionNodeLogic(*this, compositionLogicObjectType, context);
}

NodeInstance* NodeType::createInstanceLogic(const CompositionNodeLogic& comp, script::Context& context) const {
	PROFILE();
	NodeInstance* ret=
		NodeFactory::createNodeInstanceNativeLogic(nativeInstanceClassAttribute.get());
	ret->setCompositionNodeLogic(comp);
	ret->setType(*this);
	return ret;
}

void NodeType::tryStartReloading(){
	if (getResourceState() != State::Uninit){
		setResourceState(State::Unloaded); // Reload
	}
}

} // nodes
} // clover
