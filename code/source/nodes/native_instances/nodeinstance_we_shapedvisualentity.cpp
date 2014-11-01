#include "nodeinstance_we_shapedvisualentity.hpp"
#include "resources/cache.hpp"
#include "visual/entity_def.hpp"
#include "visual/entitylogic.hpp"
#include "visual/entitylogic_compound.hpp"

namespace clover {
namespace nodes {

void WeShapedVisualEntityNodeInstance::create(){
	activeInput= addInputSlot<SignalType::Boolean>("active");
	transformInput= addInputSlot<SignalType::SrtTransform3>("transform");
	materialInput= addInputSlot<SignalType::String>("material");
	shapeInput= addInputSlot<SignalType::Shape>("shape");
	shadowCastingInput= addInputSlot<SignalType::Boolean>("shadowCasting");
	
	activeInput->setOnReceiveCallback([&] (){
		entity.setActive(activeInput->get());
	});
	
	// Always receive signal before first update
	materialInput->setValueReceived();
	materialInput->setOnReceiveCallback([&] (){
		model.setMaterial(materialInput->get());
	});
	
	transformInput->setOnReceiveCallback([&] (){
		entity.setTransform(transformInput->get());
	});
	
	shapeInput->setOnReceiveCallback([&] (){
		mesh.clear();
		util::DynArray<util::Polygon> polys= shapeInput->get().get().asPolygons(0.01);
		for (const util::Polygon& poly : polys){
			mesh.add(poly.triangulated().converted<visual::TriMesh>());
		}
		util::UniformUv uv;
		mesh.applyUniformUv(uv);
	});
	
	shadowCastingInput->setOnReceiveCallback([&] (){
		def.setShadowCasting(shadowCastingInput->get());
	});
	
	model.setMesh(mesh);
	def.setModel(model);
	entity.setDef(def);
}

} // nodes
} // clover
