#include "nodeinstance_we_shapedvisualentity.hpp"
#include "resources/cache.hpp"
#include "visual/entity_def.hpp"
#include "visual/entitylogic.hpp"
#include "visual/entitylogic_compound.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* WeShapedVisualEntityNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("active", SignalType::Boolean, true);
	n->addInputSlot("transform", SignalType::SrtTransform3);
	n->addInputSlot("material", SignalType::String);
	n->addInputSlot("shape", SignalType::Shape);
	n->addInputSlot("shadowCasting", SignalType::Boolean, false);
	return n;
}

void WeShapedVisualEntityNodeInstance::create(){
	activeInput= addInputSlot<SignalType::Boolean>("active");
	transformInput= addInputSlot<SignalType::SrtTransform3>("transform");
	materialInput= addInputSlot<SignalType::String>("material");
	shapeInput= addInputSlot<SignalType::Shape>("shape");
	shadowCastingInput= addInputSlot<SignalType::Boolean>("shadowCasting");
	
	activeInput->setOnReceiveCallback(+[] (WeShapedVisualEntityNodeInstance* self){
		self->entity.setActive(self->activeInput->get());
	});
	
	// Always receive signal before first update
	materialInput->setValueReceived();
	materialInput->setOnReceiveCallback(+[] (WeShapedVisualEntityNodeInstance* self){
		self->model.setMaterial(self->materialInput->get());
	});
	
	transformInput->setOnReceiveCallback(+[] (WeShapedVisualEntityNodeInstance* self){
		self->entity.setTransform(self->transformInput->get());
	});
	
	shapeInput->setOnReceiveCallback(+[] (WeShapedVisualEntityNodeInstance* self){
		self->mesh.clear();
		util::DynArray<util::Polygon> polys= self->shapeInput->get().get().asPolygons(0.01);
		for (const util::Polygon& poly : polys){
			self->mesh.add(poly.triangulated().converted<visual::TriMesh>());
		}
		util::UniformUv uv;
		self->mesh.applyUniformUv(uv);
	});
	
	shadowCastingInput->setOnReceiveCallback(+[] (WeShapedVisualEntityNodeInstance* self){
		self->def.setShadowCasting(self->shadowCastingInput->get());
	});
	
	model.setMesh(mesh);
	def.setModel(model);
	entity.setDef(def);
}

} // nodes
} // clover
