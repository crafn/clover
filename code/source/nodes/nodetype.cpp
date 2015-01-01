#include "nodetype.hpp"
#include "compositionnodelogic.hpp"
#include "nodefactory.hpp"
#include "nodeinstance.hpp"
#include "resources/cache.hpp"
#include "signaltypetraits.hpp"
#include "slotidentifier.hpp"

namespace clover {
namespace nodes {

NodeType::NodeType()
		: INIT_RESOURCE_ATTRIBUTE(nameAttribute, "name", "")
		, INIT_RESOURCE_ATTRIBUTE(moduleAttribute, "module", "")
		, INIT_RESOURCE_ATTRIBUTE(classAttribute, "class", "")
{
	classAttribute.setOnChangeCallback([&] {tryStartReloading(); });
}

NodeType::~NodeType()
{ }

void NodeType::resourceUpdate(bool load, bool force)
{
	if (load || getResourceState() == State::Uninit) {
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
	CompositionNodeLogic* comp= createCompositionNodeLogic(classAttribute.get());
	ensure(comp);
	comp->setType(*this);
	return comp;
}

NodeInstance* NodeType::createInstanceLogic(const CompositionNodeLogic& comp) const
{
	PROFILE();
	NodeInstance* ret=
		createNodeInstanceNativeLogic(classAttribute.get());
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
