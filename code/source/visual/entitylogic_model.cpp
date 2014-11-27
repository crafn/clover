#include "entitylogic_model.hpp"
#include "entity_def_model.hpp"
#include "entity_mgr.hpp"
#include "global/event.hpp"
#include "util/profiling.hpp"
#include "util/tuple.hpp"

namespace clover {
namespace visual {

ModelEntityLogic::ModelEntityLogic(const ModelEntityDef& def):
	EntityLogic(def),
	useCustomDrawPriority(false),
	customDrawPriority(0){
}

ModelEntityLogic::~ModelEntityLogic(){
}

void ModelEntityLogic::setDrawPriority(DrawPriority p){
	useCustomDrawPriority= true;
	customDrawPriority= p;
}

ModelEntityLogic::DrawPriority ModelEntityLogic::getDrawPriority() const {
	if (useCustomDrawPriority)
		return customDrawPriority;
	
	return getDef().getDrawPriority();
}

void ModelEntityLogic::apply(const EntityLogic& other){
	EntityLogic::apply(other);
	
	if (other.getDef().getType() == EntityDef::Type::Model){
		auto o= static_cast<const ModelEntityLogic*>(&other);
		useCustomDrawPriority= o->useCustomDrawPriority;
		customDrawPriority= o->useCustomDrawPriority;
		colorMul= o->colorMul;
	}
}

util::BoundingBox<util::Vec3d> ModelEntityLogic::getBoundingBox() const {
	PROFILE();
	
	if (!getDef().getModel())
		return BoundingBox::zero();

	ensure(getDef().getModel());

	auto model_bb= getDef().getModel()->getBoundingBox();
	if (!model_bb.isSet())
		return BoundingBox::zero();
	
	/// @todo More precise solution
	
	std::array<util::Vec3f, 4> bb_v= {
		model_bb.getMin(),
		model_bb.getMax(),
		util::Vec3f{	model_bb.getMin().x,
						model_bb.getMax().y,
						model_bb.getMin().z },
		util::Vec3f{	model_bb.getMax().x,
						model_bb.getMin().y,
						model_bb.getMax().z }
	};

	util::Vec3d rad= util::Vec3d(1.0)*sqrt(	std::max({	bb_v[0].lengthSqr(),
											bb_v[1].lengthSqr(),
											bb_v[2].lengthSqr(),
											bb_v[3].lengthSqr()})*
								getDef().getOffset().scale.lengthSqr()*
								getTransform().scale.lengthSqr());
	
	return util::BoundingBox<util::Vec3d>(-rad, rad);
}

uint32 ModelEntityLogic::getContentHash() const {
	return	util::hash32(
				util::makeTuple(
					getDef().getContentHash(),
					customDrawPriority,
					useCustomDrawPriority,
					colorMul,
					getTransform()
				)
			);
}

uint32 ModelEntityLogic::getBatchCompatibilityHash() const {
	return	util::hash32(
				util::makeTuple(
					getDef().getBatchCompatibilityHash(),
					customDrawPriority,
					useCustomDrawPriority,
					static_cast<int32>(getLayer())
				)
			);
}

} // visual
} // clover
