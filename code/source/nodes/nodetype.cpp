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

void updNativeNodeInst(NodeInstance* inst)
{ return inst->update(); }

uint32 NodeType::invalidCount= 0;

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
	if (	getResourceState() == State::Error ||
			getResourceState() == State::Unloaded) {
		ensure(invalidCount > 0);
		--invalidCount;
	}

	if (load || getResourceState() == State::Uninit) {
		newNodeInst= nullptr;
		newNodeComp= nullptr;
		updNodeInst= nullptr;

#define NODEINSTANCE(x) \
		if (classAttribute.get() == util::TypeStringTraits<x>::type()) { \
			newNodeInst= newNativeNodeInst<x>; \
			newNodeComp= x::compNode; \
			updNodeInst= updNativeNodeInst; \
		}
#	include "native_instances/native_instances.def"
#undef NODEINSTANCE
		if (!newNodeInst && !newNodeComp) {
			print(	debug::Ch::Resources,
					debug::Vb::Trivial,
					"Loading NodeType: %s",
					nameAttribute.get().cStr());
			auto&& module=
				global::g_env.resCache->getResource<global::Module>(
					moduleAttribute.get());
			moduleChangeListener.clear();
			moduleChangeListener.listen(module, [this] ()
			{ resourceUpdate(false); }); // Reload on change

			newNodeInst=
				(NewNodeInst*)module.getSym(
						("new_inst_" + classAttribute.get()).cStr());
			newNodeComp=
				(NewNodeComp*)module.getSym(
						("new_comp_" + classAttribute.get()).cStr());
			updNodeInst=
				(UpdNodeInst*)module.getSym(
						("update_" + classAttribute.get()).cStr());
		}

		if (!newNodeInst || !newNodeComp || !updNodeInst) {
			++invalidCount;
			createErrorResource();
			print(	debug::Ch::Resources,
					debug::Vb::Critical,
					"Couldn't find a way to create node: %s",
					classAttribute.get().cStr());
		} else {
			setResourceState(State::Loaded);
		}
	} else {
		print(	debug::Ch::Resources,
				debug::Vb::Trivial,
				"Unloading NodeType: %s",
				nameAttribute.get().cStr());
		++invalidCount;
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

void NodeType::updateInstance(NodeInstance& inst) const
{
	ensure(updNodeInst);
	updNodeInst(&inst);
}

void NodeType::tryStartReloading()
{
	if (getResourceState() != State::Uninit)
		resourceUpdate(false);
}

} // nodes
} // clover
