#include "ec_nodes.hpp"
#include "resources/cache.hpp"
#include "nodes/nodetype.hpp"

namespace clover {
namespace game { namespace editor {

util::UniquePtr<nodes::NodeInstanceGroup> NodesEc::instantiateNodeGroup(){
	ensure(getSelectedResource());
	return getSelectedResource()->instantiate();
}

util::DynArray<util::Str8> NodesEc::getNodeTypes(){
	util::DynArray<nodes::NodeType*> nodetypes= global::g_env.resCache->getSubCache<nodes::NodeType>().getResources();
	util::DynArray<util::Str8> ret;
	for (auto& m : nodetypes){
		ret.pushBack(m->getName());
	}
	return ret;
}

nodes::CompositionNodeLogic& NodesEc::addCompositionNodeLogic(const util::Str8& logic_name){
	ensure(getSelectedResource());
	return getSelectedResource()->add(global::g_env.resCache->getResource<nodes::NodeType>(logic_name));
}

void NodesEc::removeCompositionNodeLogic(const nodes::CompositionNodeLogic& node){
	ensure(getSelectedResource());
	getSelectedResource()->remove(node);
}

}} // game::editor
} // clover
