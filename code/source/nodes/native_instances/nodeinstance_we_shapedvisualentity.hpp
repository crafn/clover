#ifndef CLOVER_NODES_NODEINSTANCE_WE_SHAPEDVISUALENTITY_HPP
#define CLOVER_NODES_NODEINSTANCE_WE_SHAPEDVISUALENTITY_HPP

#include "../nodeinstance.hpp"
#include "build.hpp"
#include "visual/entity.hpp"
#include "visual/entity_def_model.hpp"
#include "visual/mesh.hpp"

namespace clover {
namespace nodes {

class WeShapedVisualEntityNodeInstance : public NodeInstance {
public:
	static CompNode* compNode();

	virtual ~WeShapedVisualEntityNodeInstance(){}
	
	virtual void create();
	
private:
	InputSlot<SignalType::Boolean>* activeInput;
	InputSlot<SignalType::SrtTransform3>* transformInput;
	InputSlot<SignalType::String>* materialInput;
	InputSlot<SignalType::Shape>* shapeInput;
	InputSlot<SignalType::Boolean>* shadowCastingInput;
	
	visual::Entity entity;
	visual::ModelEntityDef def;
	visual::Model model;
	visual::TriMesh mesh;
};

} // nodes
namespace util {

template <>
struct TypeStringTraits<nodes::WeShapedVisualEntityNodeInstance> {
	static util::Str8 type(){ return "::WeShapedVisualEntityNodeInstance"; }
};

} // util
} // clover

#endif // CLOVER_NODES_NODEINSTANCE_WE_SHAPEDVISUALENTITY_HPP
