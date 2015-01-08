#include "nodetype.hpp"
#include "compositionnodelogic.hpp"
#include "global/module.hpp"
#include "nodeinstance.hpp"
#include "resources/cache.hpp"
#include "signaltypetraits.hpp"
#include "slotidentifier.hpp"
#include "util/profiling.hpp"

#define NODEINSTANCE_HEADERS
#include "native_instances/native_instances.def"
#undef NODEINSTANCE_HEADERS

namespace clover {
namespace nodes {

template <typename T>
NodeInstance* newNativeNodeInst()
{ return new T{}; }

NodeType::NodeType()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(moduleAttribute, "module", "")
		, INIT_RESOURCE_ATTRIBUTE(classAttribute, "class", "")
		, newNodeInst(nullptr)
		, newNodeComp(nullptr)
{
	classAttribute.setOnChangeCallback([&] { tryStartReloading(); });
	moduleAttribute.setOnChangeCallback([&] { tryStartReloading(); });
}

NodeType::~NodeType()
{ }

void NodeType::resourceUpdate(bool load, bool force)
{
	if (load || getResourceState() == State::Uninit) {
		newNodeInst= nullptr;
		newNodeComp= nullptr;

#define NODEINSTANCE(x) \
		if (classAttribute.get() == util::TypeStringTraits<x>::type()) { \
			newNodeInst= newNativeNodeInst<x>; \
			newNodeComp= x::compNode; \
		}
#	include "native_instances/native_instances.def"
#undef NODEINSTANCE
		if (!newNodeInst && !newNodeComp) {
			auto&& module=
				global::g_env.resCache->getResource<global::Module>(
					moduleAttribute.get());
			newNodeInst=
				(NewNodeInst*)module.getSym(
						("new_inst_" + classAttribute.get()).cStr());
			newNodeComp=
				(NewNodeComp*)module.getSym(
						("new_comp_" + classAttribute.get()).cStr());
		}

		if (!newNodeInst || !newNodeComp) {
			throw global::Exception(
					"Couldn't find a way to create node: %s",
					classAttribute.get().cStr());
		}

		setResourceState(State::Loaded);
	} else {
		setResourceState(State::Unloaded);
	}
}

void NodeType::createErrorResource(){
	setResourceState(State::Error);
}

CompositionNodeLogic* NodeType::createCompositionLogic() const
{
	ensure(newNodeComp);
	CompositionNodeLogic* comp= NONULL(newNodeComp());
	comp->setType(*this);
	return comp;
}

NodeInstance* NodeType::createInstanceLogic(const CompositionNodeLogic& comp) const
{
	PROFILE();
	ensure(newNodeInst);
	NodeInstance* ret= NONULL(newNodeInst());
	ret->setCompositionNodeLogic(comp);
	ret->setType(*this);
	return ret;
}

void NodeType::tryStartReloading()
{
	if (getResourceState() != State::Uninit)
		setResourceState(State::Unloaded); // Reload
}

} // nodes
} // clover
