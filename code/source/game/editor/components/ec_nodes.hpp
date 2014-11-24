#ifndef CLOVER_GAME_EDITOR_EC_NODES_HPP
#define CLOVER_GAME_EDITOR_EC_NODES_HPP

#include "build.hpp"
#include "ec_resource.hpp"
#include "nodes/compositionnodelogicgroup.hpp"
#include "nodes/nodeinstancegroup.hpp"

namespace clover {
namespace game { namespace editor {

class NodesEc;

template <>
struct EditorComponentTraits<NodesEc> {
	static const util::Str8 name(){ return "Nodes"; }
};

class NodesEc : public ResourceEc<nodes::CompositionNodeLogicGroup> {
public:
	DECLARE_EDITORCOMPONENT(NodesEc);
	
	virtual ~NodesEc(){}
	
	util::UniquePtr<nodes::NodeInstanceGroup> instantiateNodeGroup();
	
	util::DynArray<util::Str8> getNodeTypes();
	
	nodes::CompositionNodeLogic& addCompositionNodeLogic(const util::Str8& logic_name);
	void removeCompositionNodeLogic(const nodes::CompositionNodeLogic& node);
	
private:

};

}} // game::editor
} // clover

#endif // CLOVER_GAME_EDITOR_EC_NODES_HPP
