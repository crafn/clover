#include "batchpriorities.hpp"
#include "game/world_mgr.hpp"
#include "global/env.hpp"
#include "nodeinstance_we_visualentity.hpp"
#include "resources/cache.hpp"
#include "visual/entity_def.hpp"
#include "visual/entitylogic.hpp"
#include "visual/entitylogic_compound.hpp"
#include "visual/entitylogic_model.hpp"

namespace clover {
namespace nodes {

CompositionNodeLogic* WeVisualEntityNodeInstance::compNode()
{
	auto n= new CompositionNodeLogic{};
	n->addInputSlot("active", SignalType::Boolean, true);
	n->addInputSlot("entityDef", SignalType::String, util::Str8("dev_square_white"));
	n->addInputSlot("transform", SignalType::SrtTransform3);
	n->addInputSlot("pose", SignalType::ArmaturePose); // Only for CompoundEntities
	n->addInputSlot("colorMul", SignalType::Vec4, util::Vec4d(1.0));
	n->addInputSlot("events", SignalType::EventArray);

	n->setBatched(true);
	n->setBatchPriority(nodeBatchPriority_visualEntity);
	return n;
}

void WeVisualEntityNodeInstance::create(){
	entityInput= addInputSlot<SignalType::String>("entityDef");
	activeInput= addInputSlot<SignalType::Boolean>("active");
	transformInput= addInputSlot<SignalType::SrtTransform3>("transform");
	poseInput= addInputSlot<SignalType::ArmaturePose>("pose");
	colorMulInput= addInputSlot<SignalType::Vec4>("colorMul");
	eventsInput= addInputSlot<SignalType::EventArray>("events");
	
	activeInput->setOnReceiveCallback(+[] (WeVisualEntityNodeInstance* self){
		self->entity->setActive(self->activeInput->get());
	});
	
	// Always receive signal before first update
	entityInput->setValueReceived();
	entityInput->setOnReceiveCallback(+[] (WeVisualEntityNodeInstance* self){
		self->entity->changeDef(self->entityInput->get());
	});
	
	transformInput->setOnReceiveCallback(+[] (WeVisualEntityNodeInstance* self){
		self->entity->setTransform(self->transformInput->get());
	});
	
	colorMulInput->setOnReceiveCallback(+[] (WeVisualEntityNodeInstance* self){
		if (	self->entity->getDef() && self->entity->getLogic() &&
				self->entity->getDef()->getType() == visual::EntityDef::Type::Model){
			
			util::Vec4f c= self->colorMulInput->get().casted<util::Vec4f>();
			
			static_cast<visual::ModelEntityLogic*>(self->entity->getLogic())
				->setColorMul(util::Color{c.x, c.y, c.z, c.w});
		}
	});
	
	poseInput->setOnReceiveCallback(+[] (WeVisualEntityNodeInstance* self){
		const visual::EntityDef* def= self->entity->getDef();
		visual::EntityLogic* logic= self->entity->getLogic();
		
		if (def && logic && def->getType() == visual::EntityDef::Type::Compound){
			auto compound_logic= static_cast<visual::CompoundEntityLogic*>(logic);
			if (self->poseInput->get().getArmature().getJoints().size() > 0){
				compound_logic->setPose(self->poseInput->get());
			}
			else {
				print(debug::Ch::Nodes, debug::Vb::Moderate, "WeVisualEntityNodeInstance received an empty pose");
			}
		}
	});

	eventsInput->setOnReceiveCallback(+[] (WeVisualEntityNodeInstance* self){
		for (auto&& e : self->eventsInput->get()){
			if (e.getName() == "highlight"){
				self->highlightLerp= 1.0;
				self->setUpdateNeeded(true);
			}
		}
	});
}

void WeVisualEntityNodeInstance::update()
{
	updateHighlight(entity, getInputColorMul(), highlightLerp);

	highlightLerp -= global::g_env.worldMgr->getDeltaTime()*10.0;
	if (highlightLerp <= 0.0){
		setUpdateNeeded(false);
		updateHighlight(entity, getInputColorMul(), 0.0);
	}
}

void WeVisualEntityNodeInstance::updateHighlight(
		visual::Entity& entity,
		util::Color normal_c,
		real32 mul){
	if (!entity.getDef() || !entity.getLogic())
		return;

	/// @todo setColorMul could be a member of visual::EntityLogic
	auto type= entity.getDef()->getType();
	if (type == visual::EntityDef::Type::Model){
		// Highlight should be invisible when there's no light
		// so environment can't be scanned using cursor
		util::Color hl_c= util::Color{2.0, 2.0, 0.5, normal_c.a};

		auto logic= static_cast<visual::ModelEntityLogic*>(entity.getLogic());
		logic->setColorMul(util::lerp(normal_c, hl_c, mul));
	}
	else if (type == visual::EntityDef::Type::Compound){
		auto logic= static_cast<visual::CompoundEntityLogic*>(entity.getLogic());

		for (visual::Entity* e : logic->getEntities())
			updateHighlight(*NONULL(e), normal_c, mul);
	}
}

util::Color WeVisualEntityNodeInstance::getInputColorMul() const {
	return colorMulInput->get().casted<util::Vec4f>();
}

} // nodes
} // clover
